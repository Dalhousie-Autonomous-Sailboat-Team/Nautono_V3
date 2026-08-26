#include "board_io.h"
#include "test_harness.h"

I2C_HandleTypeDef hi2c2 = {2};
UART_HandleTypeDef huart1 = {1};
UART_HandleTypeDef huart2 = {2};
UART_HandleTypeDef huart3 = {3};
UART_HandleTypeDef huart4 = {4};
UART_HandleTypeDef huart5 = {5};
UART_HandleTypeDef huart6 = {6};
UART_HandleTypeDef huart7 = {7};
UART_HandleTypeDef huart8 = {8};

osMessageQueueId_t uart_rx_queueHandle = (osMessageQueueId_t)(uintptr_t)10U;
osMutexId_t debugPrintStringMutexHandle = (osMutexId_t)(uintptr_t)11U;
osSemaphoreId_t i2c2_semaphoreHandle = (osSemaphoreId_t)(uintptr_t)12U;
osSemaphoreId_t radio_tx_semaphoreHandle = (osSemaphoreId_t)(uintptr_t)13U;
osSemaphoreId_t raspberry_tx_semaphoreHandle = (osSemaphoreId_t)(uintptr_t)14U;

extern uint8_t uart4_rx_byte;
extern uint8_t uart8_rx_byte;

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *handle);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *handle);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *handle);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *handle);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *handle);

static HAL_StatusTypeDef i2c_transmit_status;
static HAL_StatusTypeDef i2c_receive_status;
static HAL_StatusTypeDef uart_transmit_it_status;
static osStatus_t semaphore_acquire_status;

static I2C_HandleTypeDef *last_i2c_handle;
static uint16_t last_i2c_address;
static uint8_t *last_i2c_buffer;
static uint16_t last_i2c_length;
static osSemaphoreId_t last_semaphore;
static uint32_t last_semaphore_timeout;
static unsigned int semaphore_acquire_calls;
static unsigned int semaphore_release_calls;
static unsigned int mutex_acquire_calls;
static unsigned int mutex_release_calls;

static UART_HandleTypeDef *receive_handles[8];
static unsigned int receive_calls;
static UART_HandleTypeDef *last_uart_transmit_handle;
static uint8_t *last_uart_transmit_buffer;
static uint16_t last_uart_transmit_length;
static unsigned int uart_transmit_it_calls;
static unsigned int uart_transmit_calls;

static UART_Char_t queued_character;
static unsigned int queue_put_calls;

static void reset_fakes(void)
{
    i2c_transmit_status = HAL_OK;
    i2c_receive_status = HAL_OK;
    uart_transmit_it_status = HAL_OK;
    semaphore_acquire_status = osOK;
    last_i2c_handle = NULL;
    last_i2c_address = 0U;
    last_i2c_buffer = NULL;
    last_i2c_length = 0U;
    last_semaphore = NULL;
    last_semaphore_timeout = 0U;
    semaphore_acquire_calls = 0U;
    semaphore_release_calls = 0U;
    mutex_acquire_calls = 0U;
    mutex_release_calls = 0U;
    receive_calls = 0U;
    last_uart_transmit_handle = NULL;
    last_uart_transmit_buffer = NULL;
    last_uart_transmit_length = 0U;
    uart_transmit_it_calls = 0U;
    uart_transmit_calls = 0U;
    memset(&queued_character, 0, sizeof(queued_character));
    queue_put_calls = 0U;
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit_IT(I2C_HandleTypeDef *handle,
                                             uint16_t address,
                                             uint8_t *data,
                                             uint16_t length)
{
    last_i2c_handle = handle;
    last_i2c_address = address;
    last_i2c_buffer = data;
    last_i2c_length = length;
    return i2c_transmit_status;
}

HAL_StatusTypeDef HAL_I2C_Master_Receive_IT(I2C_HandleTypeDef *handle,
                                            uint16_t address,
                                            uint8_t *data,
                                            uint16_t length)
{
    last_i2c_handle = handle;
    last_i2c_address = address;
    last_i2c_buffer = data;
    last_i2c_length = length;
    return i2c_receive_status;
}

HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *handle,
                                     uint8_t *data,
                                     uint16_t length)
{
    (void)data;
    (void)length;
    if (receive_calls < 8U)
        receive_handles[receive_calls] = handle;
    receive_calls++;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *handle,
                                      uint8_t *data,
                                      uint16_t length)
{
    last_uart_transmit_handle = handle;
    last_uart_transmit_buffer = data;
    last_uart_transmit_length = length;
    uart_transmit_it_calls++;
    return uart_transmit_it_status;
}

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *handle,
                                   uint8_t *data,
                                   uint16_t length,
                                   uint32_t timeout)
{
    (void)data;
    (void)length;
    (void)timeout;
    last_uart_transmit_handle = handle;
    uart_transmit_calls++;
    return HAL_OK;
}

osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout)
{
    last_semaphore = semaphore_id;
    last_semaphore_timeout = timeout;
    semaphore_acquire_calls++;
    return semaphore_acquire_status;
}

osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id)
{
    last_semaphore = semaphore_id;
    semaphore_release_calls++;
    return osOK;
}

osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
    (void)mutex_id;
    (void)timeout;
    mutex_acquire_calls++;
    return osOK;
}

osStatus_t osMutexRelease(osMutexId_t mutex_id)
{
    (void)mutex_id;
    mutex_release_calls++;
    return osOK;
}

osStatus_t osMessageQueuePut(osMessageQueueId_t queue_id,
                             const void *message_ptr,
                             uint8_t message_priority,
                             uint32_t timeout)
{
    (void)queue_id;
    (void)message_priority;
    (void)timeout;
    queued_character = *(const UART_Char_t *)message_ptr;
    queue_put_calls++;
    return osOK;
}

static void i2c_write_waits_for_completion(void)
{
    uint8_t data[2] = {1U, 2U};
    reset_fakes();

    TEST_EXPECT_TRUE(BoardI2C_Write(0xE0U, data, 2U, 25U));
    TEST_EXPECT_PTR(&hi2c2, last_i2c_handle);
    TEST_EXPECT_INT(0xE0U, last_i2c_address);
    TEST_EXPECT_PTR(data, last_i2c_buffer);
    TEST_EXPECT_INT(2U, last_i2c_length);
    TEST_EXPECT_INT(1U, semaphore_acquire_calls);
    TEST_EXPECT_PTR(i2c2_semaphoreHandle, last_semaphore);
    TEST_EXPECT_INT(25U, last_semaphore_timeout);
}

static void i2c_write_reports_hal_and_timeout_failures(void)
{
    uint8_t data = 1U;
    reset_fakes();
    i2c_transmit_status = HAL_ERROR;
    TEST_EXPECT_FALSE(BoardI2C_Write(0xE0U, &data, 1U, 10U));
    TEST_EXPECT_INT(0U, semaphore_acquire_calls);

    reset_fakes();
    semaphore_acquire_status = osErrorTimeout;
    TEST_EXPECT_FALSE(BoardI2C_Write(0xE0U, &data, 1U, 10U));
}

static void i2c_read_waits_for_completion(void)
{
    uint8_t buffer[2] = {0};
    reset_fakes();

    TEST_EXPECT_TRUE(BoardI2C_Read(0x6CU, buffer, 2U, 40U));
    TEST_EXPECT_PTR(&hi2c2, last_i2c_handle);
    TEST_EXPECT_INT(0x6CU, last_i2c_address);
    TEST_EXPECT_PTR(buffer, last_i2c_buffer);
    TEST_EXPECT_INT(40U, last_semaphore_timeout);
}

static void i2c_callbacks_release_only_the_matching_semaphore(void)
{
    I2C_HandleTypeDef other = {99};
    reset_fakes();

    HAL_I2C_MasterTxCpltCallback(&other);
    TEST_EXPECT_INT(0U, semaphore_release_calls);
    HAL_I2C_MasterTxCpltCallback(&hi2c2);
    HAL_I2C_MasterRxCpltCallback(&hi2c2);
    HAL_I2C_ErrorCallback(&hi2c2);

    TEST_EXPECT_INT(3U, semaphore_release_calls);
    TEST_EXPECT_PTR(i2c2_semaphoreHandle, last_semaphore);
}

static void uart_init_arms_expected_receive_ports(void)
{
    reset_fakes();
    BoardUART_Init();

    TEST_EXPECT_INT(4U, receive_calls);
    TEST_EXPECT_PTR(&huart4, receive_handles[0]);
    TEST_EXPECT_PTR(&huart3, receive_handles[1]);
    TEST_EXPECT_PTR(&huart8, receive_handles[2]);
    TEST_EXPECT_PTR(&huart7, receive_handles[3]);
}

static void radio_send_uses_uart8_and_releases_on_start_failure(void)
{
    reset_fakes();
    TEST_EXPECT_FALSE(Radio_Send(NULL));
    TEST_EXPECT_FALSE(Radio_Send(""));

    TEST_EXPECT_TRUE(Radio_Send("abc"));
    TEST_EXPECT_PTR(&huart8, last_uart_transmit_handle);
    TEST_EXPECT_INT(3U, last_uart_transmit_length);
    TEST_EXPECT_PTR(radio_tx_semaphoreHandle, last_semaphore);

    reset_fakes();
    uart_transmit_it_status = HAL_ERROR;
    TEST_EXPECT_FALSE(Radio_Send("abc"));
    TEST_EXPECT_INT(1U, semaphore_release_calls);
}

static void rpi_send_uses_uart7(void)
{
    reset_fakes();
    TEST_EXPECT_TRUE(RPi_Send("data"));
    TEST_EXPECT_PTR(&huart7, last_uart_transmit_handle);
    TEST_EXPECT_INT(4U, last_uart_transmit_length);
    TEST_EXPECT_PTR(raspberry_tx_semaphoreHandle, last_semaphore);
}

static void uart_receive_callback_queues_radio_byte_and_rearms_receive(void)
{
    reset_fakes();
    uart8_rx_byte = 0x5AU;

    HAL_UART_RxCpltCallback(&huart8);

    TEST_EXPECT_INT(1U, queue_put_calls);
    TEST_EXPECT_INT(UART_PORT_8, queued_character.port);
    TEST_EXPECT_INT(0x5AU, queued_character.data);
    TEST_EXPECT_INT(1U, receive_calls);
    TEST_EXPECT_PTR(&huart8, receive_handles[0]);
}

static void uart_transmit_callback_releases_correct_semaphore(void)
{
    reset_fakes();
    HAL_UART_TxCpltCallback(&huart8);
    TEST_EXPECT_PTR(radio_tx_semaphoreHandle, last_semaphore);
    HAL_UART_TxCpltCallback(&huart7);
    TEST_EXPECT_PTR(raspberry_tx_semaphoreHandle, last_semaphore);
    TEST_EXPECT_INT(2U, semaphore_release_calls);
}

static void debug_print_serializes_characters_on_uart4(void)
{
    reset_fakes();
    Debug_Print("abc");

    TEST_EXPECT_INT(1U, mutex_acquire_calls);
    TEST_EXPECT_INT(1U, mutex_release_calls);
    TEST_EXPECT_INT(3U, uart_transmit_calls);
    TEST_EXPECT_PTR(&huart4, last_uart_transmit_handle);
}

int main(void)
{
    TestHarness_Run("I2C write waits for completion", i2c_write_waits_for_completion);
    TestHarness_Run("I2C write reports failures", i2c_write_reports_hal_and_timeout_failures);
    TestHarness_Run("I2C read waits for completion", i2c_read_waits_for_completion);
    TestHarness_Run("I2C callbacks release matching semaphore",
                    i2c_callbacks_release_only_the_matching_semaphore);
    TestHarness_Run("UART init arms expected ports", uart_init_arms_expected_receive_ports);
    TestHarness_Run("radio send uses UART8", radio_send_uses_uart8_and_releases_on_start_failure);
    TestHarness_Run("RPi send uses UART7", rpi_send_uses_uart7);
    TestHarness_Run("UART receive callback queues radio byte",
                    uart_receive_callback_queues_radio_byte_and_rearms_receive);
    TestHarness_Run("UART transmit callback releases semaphore",
                    uart_transmit_callback_releases_correct_semaphore);
    TestHarness_Run("debug print uses UART4", debug_print_serializes_characters_on_uart4);
    return TestHarness_Finish();
}
