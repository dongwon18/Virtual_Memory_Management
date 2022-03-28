/**
 * Copyright (C) 2020 by Dongwon Kim, All rights reserved.
 *
 * Project 3 of Operating System: Virtual Memory Management
 *
 * File name : virtual_memory_management.c
 *
 * Summery of this code:
 *    1. get keyboard input
 *    2. if the input == random -> create and use random value for test
 *       if the input is file name -> get data from the file and use it for test
 *    3. test MIN, FIFO, LRU, LFU, Clock, WS virtual memory management
 *    4. print page fault information, brief data at console
 *    5. print residence set, page fault of each time at output file
 *
 * Written by Dongwon Kim on December 02, 2020
 *
 *  Modification History :
 *    1. Written by Dongwon Kim on December 02, 2020
 *
 *
 *  Compiler Used : gcc version 7.3.0, 32-bit
 *  Encoding : UTF - 8
 *  IDE used :Code Block 17.12
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define FILE_LENGTH 100  // keyboard input length limit
#define PAGE 100         // limit of no. of page
#define FRAME 20         // limit of no. of page frame allocated
#define WINDOW 100       // limit of window size
#define S_LEN 100000     // limit of length of reference string
#define FALSE -1

/*
 * function prototypes
 */
void VM_Test(int page, int page_frame, int window, int *string, int string_length, FILE *outfile);
// input
FILE *get_file(void);
FILE *rand_input(void);
int *get_data(FILE *infile, int *page, int *page_frame, int *window, int *string_length);
// primary functions
void mem_init(int *mem, int page_frame);
int in_mem(int *mem, int page_frame, int key);
int print_residence(int *array, int size, FILE *outfile, int time, int VA);
int find_min(int *string, int size);
int find_max(int *string, int size);
// memory management test
void MIN_test(int *mark, int *mem, int page_frame, int *string, int string_length, FILE *outfile);
void FIFO_test(int *mem, int *mark, int page_frame, int *string, int string_length, FILE *outfile);
void LRU_test(int *mem, int *mark, int page_frame, int *string, int string_length, FILE *outfile);
void LFU_test(int *mem, int *mark, int page_frame, int *string, int string_length, FILE *outfile);
void Clock_test(int *mem, int *mark, int page_frame, int *string, int string_length, FILE *outfile);
void WS_test(int page, int window, int *page_mark, int *string, int string_length, FILE *outfile);
// replace rule
int MIN_replace(int *mark, int *mem, int page_frame, int *string, int string_length, int time);
int FIFO_replace(int *mem, int *mark, int page_frame);
int LRU_replace(int *mem, int *mark, int page_frame);
int LFU_replace(int *mem, int *mark, int *tie_break, int page_frame);
int Clock_replace(int *mem, int *mark, int page_frame);

/**
 * get input file name from user
 * get data from input file
 * call virtual memory management test function
 *
 * input: none
 * output:
 *    return int 0 (successful termination)
 *               -1 (file opening failed)
 *               -2 (file format not matched)
 *               -3 (dynamic memory allocation failed)
 */
int main(){
    // get file name from user
    FILE *infile = get_file();

    // get data from input file
    int page, page_frame, window, string_length;
    int *string = get_data(infile, &page, &page_frame, &window, &string_length);
    fclose(infile);
    FILE *outfile = fopen("output.txt", "w");
    if(outfile == NULL){
        printf("[Error] file opening failed\n");
        exit(-1);
    }

    // simulation
    VM_Test(page, page_frame, window, string, string_length, outfile);

    fclose(outfile);
    free(string);
    return 0;
}

/**
 * create int pointer array using dynamic memory allocation
 * call test functions for each algorithm
 *
 * input:
 *    int page: the no. of page
 *    int page_frame: the no. of page frame allocated to the process
 *    int window: window size (for working set)
 *    int *string: int array that contains page reference string
 *    int string_length: length of *string
 *
 * output:
 *    print residence set and no. of fault for each algorithm
 */
void VM_Test(int page, int page_frame, int window, int *string, int string_length, FILE *outfile){
    /*
     * for fixed allocation
     * int *mark: to save information for replacement
     * int *mem: to save page no. which are in the page frame
     *
     * for variable allocation
     * int *page_mark: to save loaded time of page
     *
     * all initialized as -1
     */
    int *mark = (int *)malloc(sizeof(int) * page_frame);
    int *mem = (int *)malloc(sizeof(int) * page_frame);
    int *page_mark = (int *)malloc(sizeof(int) * page);
    mem_init(mark, page_frame);
    mem_init(mem, page_frame);
    mem_init(page_mark, page);

    // test each algorithm and initialize the array
    MIN_test(mark, mem, page_frame, string, string_length, outfile);
    mem_init(mark, page_frame);
    mem_init(mem, page_frame);
    FIFO_test(mem, mark, page_frame, string, string_length, outfile);
    mem_init(mark, page_frame);
    mem_init(mem, page_frame);
    LRU_test(mem, mark, page_frame, string, string_length, outfile);
    mem_init(mark, page_frame);
    mem_init(mem, page_frame);
    LFU_test(mem, mark, page_frame, string, string_length, outfile);
    mem_init(mark, page_frame);
    mem_init(mem, page_frame);
    Clock_test(mem, mark, page_frame, string, string_length, outfile);
    mem_init(mark, page_frame);
    mem_init(mem, page_frame);
    WS_test(page, window, page_mark, string, string_length, outfile);

    free(mark);
    free(mem);
    free(page_mark);
}

/**
 * initialize pointer array
 * check if there is error in dynamic memory allocation
 *
 * input:
 *    int page_frame: the no. of page frame allocated to the process
 *
 * in/output:
 *    int *mem: pointer array that is initialized as -1.
 */
void mem_init(int *mem, int page_frame){
    if(mem == NULL){
        printf("[Error] Dynamic memory allocation failed\n");
        exit(-3);
    }

    for(int i = 0; i < page_frame; i++){
        mem[i] = -1;
    }
}

/**
 * check if the 'key' is in the array (using linear search)
 *
 * input:
 *    int *mem: the array to be searched
 *    int page_frame: the size of the array
 *    int key: the value to be found
 *
 * output:
 *    return int index of the key(if the key is in the array)
 *           int FALSE(-1) (if search failed)
 */
int in_mem(int *mem, int page_frame, int key){
    for(int i = 0; i < page_frame; i++){
        if(key == mem[i]){
            return i;
        }
    }
    return FALSE;
}

/**
 * print residence set to output file
 *
 * input:
 *    int *array: array to be printed in the file
 *    int size: size of the array
 *    int time: time of the simulator
 *    int VA: 0(fixed allocation), 1(variable allocation)
 *
 * output:
 *    FILE *outfile: pointer of output file
 *    return int 0(for FA), total no. of frame occupied throughout test(VA)
 */
int print_residence(int *array, int size, FILE *outfile, int time, int VA){
    fprintf(outfile, "[Time %d] ", time + 1);
    if(VA == 0){
        for(int i = 0; i < size; i++){
            if(array[i] != -1){
                fprintf(outfile, "%d ", array[i]);
            }
        }
        fprintf(outfile, "\n");
        return 0;
    }else{
        int frame_no = 0;
        for(int i = 0; i < size; i++){
            if(array[i] != -1){
                fprintf(outfile, "%d ", i);
                frame_no++;
            }
        }
        fprintf(outfile, "\n");
        return frame_no;
    }
}

/**
 * Test function of MIN algorithm
 * check if the page no. is in the mem
 * if page fault occurs, find mem's index to be replaced and replace the page
 * Rule: exchange page whose forward distance is maximum.
 *
 * input:
 *    int page_frame: no. of page frame allocated, size of mem, mark
 *    int *string: array that contains reference string
 *    int string_length: size of string
 *
 * output:
 *    print information at the console about page fault occurs
 *    FILE *outfile: print resident set, page fault data at the output file
 *
 * in/output:
 *    int *mark: array to save information for replacement
 *    int *mem: array to save page no. which are in the page frame
 */
void MIN_test(int *mark, int *mem, int page_frame, int *string, int string_length, FILE *outfile){
    int replace;
    int fault_no = 0;
    printf("--MIN--\n");
    fprintf(outfile, "--MIN--\n");

    // test for reference string
    for(int i = 0; i < string_length; i++){
        // page fault
        if(in_mem(mem, page_frame, string[i]) == FALSE){
            // get index to be replaced
            replace = MIN_replace(mark, mem, page_frame, string, string_length, i);
            mem[replace] = string[i];
            fault_no++;
            printf("<page fault> ");
            printf("Time %d replaced index %d no. of fault %d\n", i + 1, replace, fault_no);
            fprintf(outfile, "<page fault> Time %d replaced index %d no. of fault %d\n", i + 1, replace, fault_no);
        }
        // print residence set at output file
        print_residence(mem, page_frame, outfile, i, 0);
    }
    printf("Total no. of fault: %d/%d\n", fault_no, string_length);
    fprintf(outfile, "Total no. of fault: %d/%d\n", fault_no, string_length);
}

/**
 * find index to be replaced following MIN algorithm
 *
 * input:
 *    int *mem: array to save page no. which are in the page frame
 *    int page_frame: no. of page frame allocated, size of mem, mark
 *    int *string: array that contains reference string
 *    int string_length: size of string
 *    int time: time of the system, same as index of string + 1
 *
 * output:
 *    return int index to be replaced
 *
 * in/output:
 *    int *mark: save forward distance of page
 */
int MIN_replace(int *mark, int *mem, int page_frame, int *string, int string_length, int time){
    int change = 0;
    for(int i = 0; i < page_frame; i++){
        for(int j = time; j < string_length; j++){
            // if the page is referred again in the future, save forward distance
            if(string[j] == mem[i]){
                mark[i] = j - time;
                change = 1;
                break;
            }
        }
        // if the page is not referred again, save string_length(infinity)
        // since max forward distance = string_length - 1
        if(change != 1){
            mark[i] = string_length;
        }
        change = 0;
    }

    // find page that has maximum forward distance
    int max = mark[0];
    int max_no = 0;
    for(int i = 0; i < page_frame; i++){
        if(max < mark[i]){
            max = mark[i];
            max_no = i;
        }
    }
    return max_no;
}

/**
 * Test function of FIFO algorithm
 * check if the page no. is in the mem
 * if page fault occurs, find mem's index to be replaced and replace the page
 * Rule: exchange page that has earliest arrival time.
 *
 * input:
 *    int page_frame: no. of page frame allocated, size of mem, mark
 *    int *string: array that contains reference string
 *    int string_length: size of string
 *
 * output:
 *    print information at the console about page fault occurs
 *    FILE *outfile: print resident set, page fault data at the output file
 *
 * in/output:
 *    int *mark: array to save information for replacement
 *    int *mem: array to save page no. which are in the page frame
 */
void FIFO_test(int *mem, int *mark, int page_frame, int *string, int string_length, FILE *outfile){
    int replace;
    int fault_no = 0;
    printf("--FIFO--\n");
    fprintf(outfile, "--FIFO--\n");
    for(int i = 0; i < string_length; i++){
        // page fault
        if(in_mem(mem, page_frame, string[i]) == FALSE){
            replace = FIFO_replace(mem, mark, page_frame);
            mem[replace] = string[i];
            mark[replace] = i; // mark arrival time
            fault_no++;
            printf("<page fault> ");
            printf("Time %d replaced index %d no. of fault %d\n", i + 1, replace, fault_no);
            fprintf(outfile, "<page fault> Time %d replaced index %d no. of fault %d\n", i + 1, replace, fault_no);
        }
        print_residence(mem, page_frame, outfile, i, 0);
    }
    printf("Total no. of fault: %d/%d\n", fault_no, string_length);
    fprintf(outfile, "Total no. of fault: %d/%d\n", fault_no, string_length);
}

/**
 * find index to be replaced following FIFO algorithm
 *
 * input:
 *    int *mem: array to save page no. which are in the page frame
 *    int *mark: array that has arrival time of pages in the memory
 *    int page_frame: no. of page frame allocated, size of mem, mark
 *
 * output:
 *    return int index to be replaced(index that has minimum arrival time)
 */
int FIFO_replace(int *mem, int *mark, int page_frame){
    int min = mark[0];
    int min_no = 0;
    for(int i = 0; i < page_frame; i++){
        if(min > mark[i]){
            min = mark[i];
            min_no = i;
        }
    }
    return min_no;
}

/**
 * Test function of LRU algorithm
 * check if the page no. is in the mem
 * if page fault occurs, find mem's index to be replaced and replace the page
 * Rule: exchange page that has earliest used time.
 *
 * input:
 *    int page_frame: no. of page frame allocated, size of mem, mark
 *    int *string: array that contains reference string
 *    int string_length: size of string
 *
 * output:
 *    print information at the console about page fault occurs
 *    FILE *outfile: print resident set, page fault data at the output file
 *
 * in/output:
 *    int *mark: array to save information for replacement
 *    int *mem: array to save page no. which are in the page frame
 */
void LRU_test(int *mem, int *mark, int page_frame, int *string, int string_length, FILE *outfile){
    int replace;
    int fault_no = 0;
    int index;
    printf("--LRU--\n");
    fprintf(outfile, "--LRU--\n");
    for(int i = 0; i < string_length; i++){
        index = in_mem(mem, page_frame, string[i]);
        // page fault
        if(index == FALSE){
            replace = LRU_replace(mem, mark, page_frame);
            mem[replace] = string[i];
            mark[replace] = i; // mark used time
            fault_no++;
            printf("<page fault> ");
            printf("Time %d replaced index %d no. of fault %d\n", i + 1, replace, fault_no);
            fprintf(outfile, "<page fault> Time %d replaced index %d no. of fault %d\n", i + 1, replace, fault_no);
        }else{
            mark[index] = i;  // mark used time
        }
        print_residence(mem, page_frame, outfile, i, 0);
    }
    printf("Total no. of fault: %d/%d\n", fault_no, string_length);
    fprintf(outfile, "Total no. of fault: %d/%d\n", fault_no, string_length);
}

/**
 * find index to be replaced following LRU algorithm
 *
 * input:
 *    int *mem: array to save page no. which are in the page frame
 *    int *mark: array that has arrival time of pages in the memory
 *    int page_frame: no. of page frame allocated, size of mem, mark
 *
 * output:
 *    return int index to be replaced(index that has minimum used time)
 */
int LRU_replace(int *mem, int *mark, int page_frame){
    int min = mark[0];
    int min_no = 0;
    for(int i = 0; i < page_frame; i++){
        if(min > mark[i]){
            min = mark[i];
            min_no = i;
        }
    }
    return min_no;
}

/**
 * Test function of LFU algorithm
 * check if the page no. is in the mem
 * if page fault occurs, find mem's index to be replaced and replace the page
 * Rule: exchange page that has smallest used count.
 *       when more than one pages have same value, use LRU for tie breaking.
 *
 * input:
 *    int page_frame: no. of page frame allocated, size of mem, mark
 *    int *string: array that contains reference string
 *    int string_length: size of string
 *
 * output:
 *    print information at the console about page fault occurs
 *    FILE *outfile: print resident set, page fault data at the output file
 *
 * in/output:
 *    int *mark: array to save information for replacement
 *    int *mem: array to save page no. which are in the page frame
 */
void LFU_test(int *mem, int *mark, int page_frame, int *string, int string_length, FILE *outfile){
    int replace;
    int fault_no = 0;
    int index;

    printf("--LFU--\n");
    fprintf(outfile, "--LFU--\n");
    // array to save used time for tie breaking
    int *tie_break = (int *)malloc(sizeof(int) * page_frame);
    mem_init(tie_break, page_frame);

    for(int i = 0; i < string_length; i++){
        index = in_mem(mem, page_frame, string[i]);
        // page fault
        if(index == FALSE){
            replace = LFU_replace(mem, mark, tie_break, page_frame);
            mem[replace] = string[i];
            mark[replace] = 1;       // mark used count
            tie_break[replace] = i;  // mark used time
            fault_no++;
            printf("<page fault> ");
            printf("Time %d replaced index %d no. of fault %d\n", i + 1, replace, fault_no);
            fprintf(outfile, "<page fault> Time %d replaced index %d no. of fault %d\n", i + 1, replace, fault_no);
        }else{
            mark[index] += 1;        //  increase used count
            tie_break[index] = i;    //  mark used time
        }
        print_residence(mem, page_frame, outfile, i, 0);
    }
    free(tie_break);

    printf("Total no. of fault: %d/%d\n", fault_no, string_length);
    fprintf(outfile, "Total no. of fault: %d/%d\n", fault_no, string_length);
}

/**
 * find index to be replaced following LFU algorithm
 *
 * input:
 *    int *mem: array to save page no. which are in the page frame
 *    int *mark: array that has used count of pages in the memory
 *    int *tie_break: array that has used time of pages in the memory
 *    int page_frame: no. of page frame allocated, size of mem, mark, tie_break
 *
 * output:
 *    return int index to be replaced(index that has minimum used count)
 */
int LFU_replace(int *mem, int *mark, int *tie_break, int page_frame){
    int min = mark[0];
    int min_no = 0;
    for(int i = 0; i < page_frame; i++){
        if(min > mark[i]){
            min = mark[i];
        }
    }
    int tie_min = tie_break[0];
    // if the page has same used count with minimum value, tie_break by LRU
    for(int i = 0; i < page_frame; i++){
        if(mark[i] == min){
            if(tie_min > tie_break[i]){
                tie_min = tie_break[i];
                min_no = i;
            }
        }
    }
    return min_no;
}

/**
 * Test function of Clock algorithm
 * check if the page no. is in the mem
 * if page fault occurs, find mem's index to be replaced and replace the page
 * Rule: exchange page whose reference bit is 0.
 *       if current page's bit is 1, change it to 0 and examine next page
 *
 * input:
 *    int page_frame: no. of page frame allocated, size of mem, mark
 *    int *string: array that contains reference string
 *    int string_length: size of string
 *
 * output:
 *    print information at the console about page fault occurs
 *    FILE *outfile: print resident set, page fault data at the output file
 *
 * in/output:
 *    int *mark: array to save information for replacement
 *    int *mem: array to save page no. which are in the page frame
 */
void Clock_test(int *mem, int *mark, int page_frame, int *string, int string_length, FILE *outfile){
    int replace;
    int fault_no = 0;
    int index;
    printf("--Clock--\n");
    fprintf(outfile, "--Clock--\n");
    for(int i = 0; i < string_length; i++){
        index = in_mem(mem, page_frame, string[i]);
        // page fault
        if(index == FALSE){
            replace = Clock_replace(mem, mark, page_frame);
            mem[replace] = string[i];
            mark[replace] = 1; // reference bit = 1
            fault_no++;
            printf("<page fault> ");
            printf("Time %d replaced index %d no. of fault %d\n", i + 1, replace, fault_no);
            fprintf(outfile, "<page fault> Time %d replaced index %d no. of fault %d\n", i + 1, replace, fault_no);
        }else{
            mark[index] = 1;   // reference bit = 1
        }
        print_residence(mem, page_frame, outfile, i, 0);
    }
    printf("Total no. of fault: %d/%d\n", fault_no, string_length);
    fprintf(outfile, "Total no. of fault: %d/%d\n", fault_no, string_length);
}

/**
 * find index to be replaced following Clock algorithm
 *
 * input:
 *    int *mem: array to save page no. which are in the page frame
 *    int *mark: array that has reference bit of pages in the memory
 *    int page_frame: no. of page frame allocated, size of mem, mark
 *
 * output:
 *    return int index to be replaced(index whose reference bit  == 0)
 */
int Clock_replace(int *mem, int *mark, int page_frame){
    // needle value should be hold for next search -> use static
    static int needle = 0;
    int replace;
    // find page whose reference bit == 0
    while(1){
        if(mark[needle] == 0){
            replace = needle;
            needle++;
            needle = needle % page_frame;
            break;
        // if reference bit == 1, change to 0 and examine next page
        }else{
            mark[needle] = 0;
            needle++;
            needle = needle % page_frame;
        }
    }
    return replace;
}

/**
 * Test function of WS algorithm
 * check if the page no. is in the mem
 * if page fault occurs, find mem's index to be replaced and replace the page
 * Rule: maintain only working set is in memory
 *
 * input:
 *    int page: no. of page of the process, size of page_mark
 *    int window: size of window
 *    int *string: array that contains reference string
 *    int string_length: size of string
 *
 * output:
 *    print information at the console about page fault occurs
 *    FILE *outfile: print resident set, page fault data at the output file
 *
 * in/output:
 *    int *page_mark: array to save used time of the page
 *                    -1 means the page is not in memory
 */
void WS_test(int page, int window, int *page_mark, int *string, int string_length, FILE *outfile){
    int fault_no = 0;
    int limit;
    int frame_no = 0;
    printf("--WS--\n");
    fprintf(outfile, "--WS--\n");

    /*
     * i = time
     * string[i] = used page no. at the time i
     * page_mark[j] = used time(index = page no.)
     */
    for(int i = 0; i < string_length; i++){
        // not in memory(page fault)
        if(page_mark[string[i]] == -1){
            page_mark[string[i]] = i; // mark loaded time
            fault_no++;
            printf("<page fault> ");
            printf("Time %d Insert %d no. of fault %d\n", i + 1, string[i], fault_no);
            fprintf(outfile, "<page fault> Time %d Insert %d no. of fault %d\n", i + 1, string[i], fault_no);
        }
        limit = i - window;
        page_mark[string[i]] = i;
        // check if the page is not in working set
        for(int j = 0; j < page; j++){
            if(limit > page_mark[j]){
                page_mark[j] = -1;
            }
        }
        // count no. of page frame used throughout test
        // since WS is variable allocation, 5th argument is 1
        frame_no += print_residence(page_mark, page, outfile, i, 1);
    }
    printf("average page frame no.: %.2lf\n", (double)frame_no / string_length);
    fprintf(outfile, "average page frame no.: %.2lf\n", (double)frame_no / string_length);
    printf("Total no. of fault: %d/%d\n", fault_no, string_length);
    fprintf(outfile, "Total no. of fault: %d/%d\n", fault_no, string_length);
}

/**
 * get input from user(maximum length: FILE_LENGTH - 1, 1 for '\0')
 *    -random: use random input and save it at rand_input.txt
 *    -file's name: open the file
 *
 * output:
 *    return FILE *infile: file pointer of input file
 */
FILE *get_file(void){
    char *filename = (char *)malloc(sizeof(char) * FILE_LENGTH);
    printf("Enter 'random'(random input) or file name to open: ");
    scanf("%s", filename);

    if(strcmp(filename, "random") == 0){
        printf("random\n");
        free(filename);
        return rand_input();
    }else{
        FILE *infile = fopen(filename, "r");
        if(infile == NULL){
            printf("[Error] File opening failed\n");
            exit(-1);
        }
        free(filename);
        return infile;
    }
}

/**
 * create random input
 * randomly choose page, page_frame, window, string, string_length within the range
 *
 * output:
 *    FILE *infile: file pointer that has randomly chosen values
 */
FILE *rand_input(void){
    srand(time(NULL));
    int page = rand() % PAGE + 1;
    int page_frame = rand() % FRAME + 1;
    int window = rand() % WINDOW + 1;
    // range: page + 1 ~ S_LEN
    int string_length = rand() % (S_LEN - page) + 1 + page;

    FILE *infile = fopen("rand_input.txt", "w+");
    fprintf(infile, "%d %d %d %d\n", page, page_frame, window, string_length);
    for(int i = 0; i < string_length; i++){
        fprintf(infile, "%d ", rand() % page);
    }
    fprintf(infile, "\n");

    // set file pointer to the start
    fseek(infile, 0, SEEK_SET);
    return infile;
}

/**
 * get data from input file
 *
 * input:
 *    FILE *infile: file pointer for input file
 *
 * output:
 *    return int *string: array that has reference string(size: string_length)
 *    int *page: no. of page of the process, size of page_mark
 *    int *page_frame: no. of page frame allocated
 *    int *window: size of window
 *    int *string_length: size of string
 */
int *get_data(FILE *infile, int *page, int *page_frame, int *window, int *string_length){
    fscanf(infile, "%d %d %d %d\n", page, page_frame, window, string_length);
    printf("%d %d %d %d\n", *page, *page_frame, *window, *string_length);

    // file format check
    if((*page > PAGE) || (*page < 0)){
        printf("[Error]The range of no. of page: 0~ 100\n");
        exit(-2);
    }
    if((*page_frame > FRAME) || (*page_frame < 0)){
        printf("[Error]The range of no. of page frame: 0~ 20\n");
        exit(-2);
    }
    if((*window > WINDOW) || (*window < 0)){
        printf("[Error]The range of window size: 0~ 100\n");
        exit(-2);
    }
    if((*string_length > S_LEN) || (*string_length < 0)){
        printf("[Error]The range of length of string: 0~ 100000\n");
        exit(-2);
    }

    int *string = (int *)malloc(sizeof(int) * (*string_length));
    if(string == NULL){
        printf("[Error] Dynamic memory allocation failed\n");
        exit(-3);
    }

    // get reference string and save to string[i]
    int cnt = 0;
    for(int i = 0; i < (*string_length); i++){
        cnt += fscanf(infile, "%d ", &(string[i]));
        printf("%d ", string[i]);
    }
    printf("\n");

    // file format check
    if(cnt != (*string_length)){
        printf("[Error] no. of page reference and string's length not matched\n");
        exit(-2);
    }

    if(find_min(string, *string_length) < 0){
        printf("[Error] page number can't be smaller than 0\n");
        exit(-2);
    }
    if(find_max(string, *string_length) > (*page - 1)){
        printf("[Error] maximum page number can't be bigger than total page no.\n");
        exit(-2);
    }

    return string;
}

int find_min(int *string, int size){
    int min = string[0];
    for(int i = 0; i < size; i++){
        if(string[i] < min){
            min = string[i];
        }
    }
    return min;
}

int find_max(int *string, int size){
    int max = string[0];
    for(int i = 0; i < size; i++){
        if(string[i] > max){
            max = string[i];
        }
    }
    return max;
}
