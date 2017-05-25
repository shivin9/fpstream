#include<stdio.h>
#include<stdlib.h>
#include <malloc.h>

#define first(d) (((d)[(0)]) + (2))
#define last(d) (((d)[(0)]) + ((d)[(1)]) + (1))

typedef int* data;
int sf_cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}

data sf_copy_data(data d)
{
    data temp = malloc((d[1]+2)*sizeof(int));
    memcpy(temp, d + first(d) - 2, (d[1] + 2)*sizeof(int)); // copy from starting position
    temp[0] = 0;
    temp[1] = d[1];
    return temp;
}

data sf_sort_data(data d) // seen
{
    // does insertion sort of items and removes the duplicate items also
    qsort(d + 2, d[1], sizeof(d[0]), sf_cmpfunc);

    /*
       Removing duplicate items here in O(n)
    */
    int ori = d[2], prev_idx, temp, new_len = 1;

    for(prev_idx = 0, temp = prev_idx + 1; temp != d[1]; temp++)
    {
        while((temp < d[1] - 1) && d[temp + 2] == ori)
        {
            temp++;
        }

        if(temp < d[1])
        {
            d[prev_idx + 3] = d[temp + 2];
            ori = d[temp + 2];
            prev_idx++;
            new_len++;
        }
    }
    data new = malloc((new_len + 2) * sizeof(int));
    memcpy(new, d, (new_len + 2)*sizeof(int));
    new[1] = new_len;

    if(new[last(new)] == new[last(new)-1])
    {
        new[last(new)] = 0;
        new[1]--;
    }
    return new;
}

void sf_print_data_node(data d)
{
    int i;
    for(i = 0; i < d[1]; i++)
    {
        printf("%d ", d[first(d) + i]);
    }
    printf("\n");
}

int main()
{
    int arr[20] = {0, 18, 89, 67, 2, 85, 9, 33, 43, 89, 10, 16, 89, 72, 18, 33, 89, 55, 9, 20};
    qsort(arr+2, arr[1], sizeof(arr[0]), sf_cmpfunc);
    int i;
    for(i = 0; i < arr[1] + 2; i++)
	   printf("%d ", arr[i]);

	data new = sf_sort_data(arr);
    printf("\n");

    for(i = 0; i < new[1] + 2; i++)
	   printf("%d ", new[i]);

    // new[0] = 4;
    // new[1] = 3;
    // printf("\nfirst(new) = %d, last(new) = %d\n", first(new), last(new));

    // data copy = sf_copy_data(new);
    // sf_print_data_node(copy);

    // printf("\n");   

    // data c1 = sf_copy_data(new);
    // sf_print_data_node(c1);

    printf("\n");	
    return 0;
}
