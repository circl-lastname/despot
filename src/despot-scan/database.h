#pragma once

#include <despot.h>
#include <stdio.h>

despot_result_t database_load(FILE* database);
despot_result_t database_save(FILE* database);
