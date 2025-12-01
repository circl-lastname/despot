#pragma once

#define EXPORT __attribute__((visibility("default")))
#define MUST(...) do { despot_result_t __temp_result = (__VA_ARGS__); if (__temp_result != DESPOT_RESULT_SUCCESS) { return __temp_result; } } while(0)

// Use these together
#define TRY(...) do { despot_result_t __temp_result = (__VA_ARGS__); if (__temp_result != DESPOT_RESULT_SUCCESS) {
#define CATCH(...) __VA_ARGS__ return __temp_result; } } while(0)
