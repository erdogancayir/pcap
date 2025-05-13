#include "../test_framework.h"
#include "../../include/cli_parser.h"
#include <string.h>
#include <stdbool.h>

void test_valid_interface_args(void) {
    char *argv[] = {"program", "-i", "eth0", "-o", "output.txt"};
    int argc = 5;
    
    cli_config_t config;
    int result = parse_cli_args(argc, argv, &config);
    
    TEST_ASSERT(result == 0, "Valid interface arguments should parse successfully");
    TEST_ASSERT(config.input_type == INPUT_TYPE_INTERFACE, "Should use interface mode");
    TEST_STRING_EQUAL(config.interface_or_file, "eth0", "Interface name should be correct");
    TEST_STRING_EQUAL(config.output_file, "output.txt", "Output file should be correct");
    
    free_config(&config);
}

void test_valid_pcap_file_args(void) {
    char *argv[] = {"program", "-r", "test.pcap", "-o", "output.txt"};
    int argc = 5;
    
    cli_config_t config;
    int result = parse_cli_args(argc, argv, &config);
    
    TEST_ASSERT(result == 0, "Valid pcap file arguments should parse successfully");
    TEST_ASSERT(config.input_type == INPUT_TYPE_PCAP_FILE, "Should use file mode");
    TEST_STRING_EQUAL(config.interface_or_file, "test.pcap", "Pcap file should be correct");
    TEST_STRING_EQUAL(config.output_file, "output.txt", "Output file should be correct");
    
    free_config(&config);
}

void test_missing_output_file(void) {
    char *argv[] = {"program", "-i", "eth0"};
    int argc = 3;
    
    cli_config_t config;
    int result = parse_cli_args(argc, argv, &config);
    
    TEST_ASSERT(result == -1, "Missing output file should fail");
}

void test_conflicting_args(void) {
    char *argv[] = {"program", "-i", "eth0", "-r", "test.pcap", "-o", "output.txt"};
    int argc = 7;
    
    cli_config_t config;
    int result = parse_cli_args(argc, argv, &config);
    
    TEST_ASSERT(result == -1, "Conflicting interface and file arguments should fail");
}

void test_invalid_args(void) {
    char *argv[] = {"program", "-x", "invalid"};
    int argc = 3;
    
    cli_config_t config;
    int result = parse_cli_args(argc, argv, &config);
    
    TEST_ASSERT(result == -1, "Invalid arguments should fail");
}

int main(void) {
    init_test_suite();
    
    test_valid_interface_args();
    test_valid_pcap_file_args();
    test_missing_output_file();
    test_conflicting_args();
    test_invalid_args();
    
    print_test_summary();
    return test_stats.failed_tests > 0 ? 1 : 0;
} 