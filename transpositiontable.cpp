#include "transpositiontable.h"

void JACEA::clear_table(std::vector<TTEntry> &table, const int size)
{
    for (int i = 0; i < size; i++)
    {
        table[i] = {0, 0, 0, 0, 0};
    }
}