#include "app_protocol.h"
#include "test_harness.h"

static const char *valid_rpi_packet =
    "{\"TargetsOutput\":[{"
    "\"targetBearing\":91.5,"
    "\"waypointLat\":44.25,"
    "\"waypointLon\":-63.75,"
    "\"targetSailAngle\":25.5,"
    "\"targetFlapAngle\":3.0,"
    "\"targetRudderAngle\":-12.5,"
    "\"latitude\":44.6,"
    "\"longitude\":-63.6,"
    "\"headingAngle\":180.0,"
    "\"windAngle\":42.25"
    "}]}";

static void string_to_float_handles_sign_and_precision(void)
{
    TEST_EXPECT_FLOAT_NEAR(226.0f, StringToFloat("226.0"), 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-12.5f, StringToFloat("-12.5"), 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(1.2345f, StringToFloat("+1.23456"), 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(0.0f, StringToFloat(NULL), 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(0.0f, StringToFloat("not-a-number"), 0.0001f);
}

static void float_to_string_rounds_to_nearest_integer(void)
{
    char buffer[16];

    FloatToString(12.6f, buffer);
    TEST_EXPECT_STRING("13", buffer);

    FloatToString(-12.6f, buffer);
    TEST_EXPECT_STRING("-13", buffer);
}

static void xbee_parser_extracts_signed_commands(void)
{
    XbeeCommand_t command = {0};

    bool parsed = AppProtocol_ParseXbeeCommand("{sa:30.5,ra:-12.25}\n", &command);

    TEST_EXPECT_TRUE(parsed);
    TEST_EXPECT_FLOAT_NEAR(30.5f, command.sail_angle, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-12.25f, command.rud_angle, 0.0001f);
}

static void xbee_parser_rejects_incomplete_packets(void)
{
    XbeeCommand_t command = {0};

    TEST_EXPECT_FALSE(AppProtocol_ParseXbeeCommand("{sa:30.5}", &command));
    TEST_EXPECT_FALSE(AppProtocol_ParseXbeeCommand("sa:30.5,ra:1}", &command));
    TEST_EXPECT_FALSE(AppProtocol_ParseXbeeCommand(NULL, &command));
    TEST_EXPECT_FALSE(AppProtocol_ParseXbeeCommand("{sa:1,ra:2}", NULL));
}

static void rpi_parser_extracts_all_fields(void)
{
    RPiSample_t sample = {0};

    bool parsed = AppProtocol_ParseRpiSample(valid_rpi_packet, &sample);

    TEST_EXPECT_TRUE(parsed);
    TEST_EXPECT_FLOAT_NEAR(91.5f, sample.target_bearing, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(44.25f, sample.target_lat, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-63.75f, sample.target_lon, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(25.5f, sample.target_sail_angle, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(3.0f, sample.target_flap_angle, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-12.5f, sample.target_rudder_angle, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(44.6f, sample.current_lat, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-63.6f, sample.current_lon, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(180.0f, sample.current_bearing, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(42.25f, sample.current_wind_angle, 0.0001f);
}

static void rpi_parser_rejects_missing_required_field(void)
{
    RPiSample_t sample = {0};
    const char *missing_wind =
        "{\"TargetsOutput\":[{"
        "\"targetBearing\":1,\"waypointLat\":2,\"waypointLon\":3,"
        "\"targetSailAngle\":4,\"targetFlapAngle\":5,\"targetRudderAngle\":6,"
        "\"latitude\":7,\"longitude\":8,\"headingAngle\":9}]}";

    TEST_EXPECT_FALSE(AppProtocol_ParseRpiSample(missing_wind, &sample));
}

static void wind_parser_extracts_iimwv_fields(void)
{
    WindSample_t sample = {0};

    bool parsed = AppProtocol_ParseWindNmea("$IIMWV,123.4,R,5.6,N,A*2F\r\n", &sample);

    TEST_EXPECT_TRUE(parsed);
    TEST_EXPECT_FLOAT_NEAR(123.4f, sample.direction, 0.0001f);
    TEST_EXPECT_INT('R', sample.reference);
    TEST_EXPECT_FLOAT_NEAR(5.6f, sample.speed, 0.0001f);
    TEST_EXPECT_INT('N', sample.speed_unit);
    TEST_EXPECT_INT('A', sample.status);
}

static void wind_parser_rejects_wrong_sentence_type(void)
{
    WindSample_t sample = {0};
    TEST_EXPECT_FALSE(AppProtocol_ParseWindNmea("$GPGGA,1,2,3", &sample));
    TEST_EXPECT_FALSE(AppProtocol_ParseWindNmea(NULL, &sample));
    TEST_EXPECT_FALSE(AppProtocol_ParseWindNmea("$IIMWV,1,R,2,N,A*00", NULL));
}

int main(void)
{
    TestHarness_Run("string-to-float handles sign and precision",
                    string_to_float_handles_sign_and_precision);
    TestHarness_Run("float-to-string rounds", float_to_string_rounds_to_nearest_integer);
    TestHarness_Run("XBee parser extracts commands", xbee_parser_extracts_signed_commands);
    TestHarness_Run("XBee parser rejects incomplete packets",
                    xbee_parser_rejects_incomplete_packets);
    TestHarness_Run("RPi parser extracts all fields", rpi_parser_extracts_all_fields);
    TestHarness_Run("RPi parser rejects missing required fields",
                    rpi_parser_rejects_missing_required_field);
    TestHarness_Run("wind parser extracts IIMWV fields", wind_parser_extracts_iimwv_fields);
    TestHarness_Run("wind parser rejects wrong sentence type",
                    wind_parser_rejects_wrong_sentence_type);
    return TestHarness_Finish();
}
