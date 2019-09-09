#include "data.h"

//Lookup table function
Type get_data_type(int cat_index);

//Merges 2 lists in sorted order
//Core of the merge sort sorting fucntion
void merge(Movie* movies, int cat_index, int l, int m, int r);

//Helper function for the merge sort sorting algorithm
void msort(Movie* movies, int cat_index, int l, int r);

//Helper function for the quick sort sorting algorithm
void qksort(Movie* movies, int cat_index, int l, int r);
