#include <esp_partition.h>
#include <esp_flash.h>



void partitioner();

void partList();

void dumpPartition(const char* partitionLabel, const char* outputPath);

void restorePartition(const char* partitionLabel);