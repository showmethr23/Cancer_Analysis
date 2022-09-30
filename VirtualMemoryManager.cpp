// VirtualMemoryManager.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <iostream>
#include "LinkedList.h"

#define EMPTY				-1
#define NUMBER_OF_FRAMES	3        //for physical memory
#define FRAME_SIZE			256      //for physical memory
#define NUMBER_OF_PAGES		256      //for logical memory
#define PAGE_SIZE			256      //for logical memory

enum OPCODE
{
	RET = 0,
	MOV = 1,
	PRN = 2
};

LinkedList* queue = NULL;
int fault_count = 0;

//How to interpret the address:
//strNum[3] instruction opcode
//strNum[2] register (not used because we have only one register)
//strNum[1] page number
//strNum[0] offset
union Address_Type {
	int  iNum;
	unsigned char strNum[4];
};

FILE* vm_file_Ptr;
errno_t err;
int program_physical_memory[FRAME_SIZE];
unsigned char* data_physical_memory[NUMBER_OF_FRAMES];
int page_table[NUMBER_OF_FRAMES];
unsigned char AX_REGISTER;
int display_x = 0, frame_index, offset;

void init_PhysicalMemory();
int loadProgram(int* buffer, int length);
bool load_LogicalToPhysical(int page_number, int frame_index);
bool is_PageLoaded(int page_numbr, int* frame_index);
void update_page_table(int page_number, int frame_index);
void decodeAddress(int address_param, OPCODE* opcode, int* page, int *offset);
int findEmptyFrame();
void execute(OPCODE opcode);
void print_page_table();

//This function removes the least recently use page and add the new page
int swapPage_LRU(int page_to_add)
{
	int local_frame_index = 0;
	//TODO> Add your logic here>
	//use functions from the queue as suggested by the book
	local_frame_index = queue->RemoveTail();
	queue->AddToHead(page_to_add, local_frame_index);

	return local_frame_index;
}

//This function reads the next instruction from the program memory program_physical_memory
//decode the instruction
//returns reads 1 byte from physical memory if the page is loaded and return its
//If the page is not loaded, find a free frame and load it
//If no free frame exist, swap out one
//To figure out which frame to swap out, it uses LRU page replacement algorithm
OPCODE fetch()
{
	OPCODE opcode=RET;
    int page;
	
	static int PC = 0;
	decodeAddress(program_physical_memory[PC++], &opcode, &page, &offset);

	if (opcode == MOV) {
		//TODO> Add your logic here>
		//The following are global variables that need to be updated here:
		//fault_count keep track of page fault
		//frame_index must be set to the current frame being use by the current instruction
		//The MOV instruction requires to read data from physical memory
		//if the data is not loaded, find an empty frame and load it
		//if no empty frame exists swap one out using LRU
		//The following functions are available to call:
		//is_PageLoaded, swapPage_LRU, queue->AddToHead, queue->MoveToHead, 
		//findEmptyFrame, update_page_table, load_LogicalToPhysical
		if (is_PageLoaded(page, &frame_index)) {    //page already in physical memory
			//update queue for LRU
			queue->MoveToHead(page);
		}
		else {  //page not in physical memory
			fault_count++;
			frame_index = findEmptyFrame();
			if (frame_index == NOT_FOUND) {   //no free spot
				frame_index = swapPage_LRU(page);
			}
			else {    //free spot exist
				queue->AddToHead(page, frame_index);
			}

			update_page_table(page, frame_index);
			load_LogicalToPhysical(page, frame_index);
		}
			
	}

	return opcode;
}

////////////////// NO NEED TO MODIFY ANY CODE BELLOW THIS POINT /////////////////////////////
int main()
{
	OPCODE opcode;
	queue = new LinkedList();
	int previous_page = -1, page_jump = 0, length;
	init_PhysicalMemory();

	std::cout << "\n\nLRU Algorithm\n";
	length = loadProgram(&program_physical_memory[0], FRAME_SIZE);
	
	//CPU Simulation
	opcode = fetch();
	while(opcode != RET)
	{
		execute(opcode);
		opcode = fetch();
	}

	std::cout << "\n\nLRU: # of Page Fault: " << fault_count << "\n";
	return 0;
}

void init_PhysicalMemory()
{
	for (int i = 0; i < NUMBER_OF_FRAMES; i++)
	{
		data_physical_memory[i] = NULL;
		page_table[i] = EMPTY;
	}
}

//This function loads data the specify page from virtual memory (disk) and copy it to physical memory
//at the specify frame index.
bool load_LogicalToPhysical(int page_number, int frame_index)
{
	if (page_number < 0 || page_number>(NUMBER_OF_PAGES - 1))
		return false;
	if (frame_index < 0 || frame_index>(NUMBER_OF_FRAMES - 1))
		return false;

	unsigned char* Ptr = (unsigned char*)malloc(PAGE_SIZE);

	err = fopen_s(&vm_file_Ptr, "BACKING_STORE.bin", "rb");
	if (vm_file_Ptr != NULL)
	{
		size_t flag = fseek(vm_file_Ptr, page_number * PAGE_SIZE, 0);
		flag = fread(Ptr, 1, PAGE_SIZE, vm_file_Ptr);
		fclose(vm_file_Ptr);
		if (data_physical_memory[frame_index] != NULL)
			free(data_physical_memory[frame_index]);
		data_physical_memory[frame_index] = Ptr;
		return true;
	}

	return false;
}

void update_page_table(int page_number, int frame_index)
{
	if (frame_index < 0 || frame_index >(NUMBER_OF_FRAMES - 1))
		return;

	page_table[frame_index] = page_number;
}

//The program is a series of instruction of the form:
//>MOV AX, [data mem address]     <this instruct the CPU to move the character from address>
//>PRN                            <this instruct the CPU to print to the screen the content of the AX register>
// ... 
//>RET                            <program ends> 
int loadProgram(int* buffer, int length)
{
	int instruction, j, ret, count = 0, page, offset;
	OPCODE opcode;
	std::cout << "Reference string\n";

	memset(buffer, 0, length);
	err = fopen_s(&vm_file_Ptr, "addresses.txt", "r");
	if (vm_file_Ptr != NULL)
	{
		for (j = 0; j < length; j++)
		{
			ret = fscanf_s(vm_file_Ptr, "%d, ", &instruction);
			if (ret > 0)
			{
				buffer[j] = instruction;
				decodeAddress(instruction, &opcode, &page, &offset);
				if (opcode == MOV)
				{
					std::cout << page << "  ";
				}
				count++;
			}
			else 
				break;
		}
		fclose(vm_file_Ptr);
	}

	std::cout << "\n";

	return count;
}

//This function takes a 32 bits number and return the corresponding page number.
//The format is "XXPO" where each of those letter are 8bits
//O is the least significant byte which represent the offset number
//P represent the page number.
//This can be implemented using the methods:
//1) using structure type "Address_Type"
//2) using bit manipulation, mask and shift (note: & is logical and, | is logical or, << is shift left, >> is shift right
// 0xFFFF is the mask to eliminate the most significant 2 bytes
void decodeAddress(int address_param, OPCODE* opcode, int* page, int* offset)
{
    //TODO in class
	{
		Address_Type address;
		address.iNum = address_param;
		*page = address.strNum[1];
		*offset = address.strNum[0];
		*opcode = (OPCODE)address.strNum[3];
	}
}

//This function determine if a page is in memory by checking the page_table
//If it exist it returns the frame index
//if not it returns NOT_FOUND
int getFrameIndex(int page)
{
	int frame_index = NOT_FOUND;

	for (int i = 0; i < NUMBER_OF_FRAMES; i++)
		if (page_table[i] == page)
		{
			frame_index = page_table[i];
			frame_index = i;
			break;
		}

	return frame_index;
}

//This function determine if a page is in memory by checking the page_table
bool is_PageLoaded(int page_numbr, int* frame_index)
{
	*frame_index = getFrameIndex(page_numbr);
	return *frame_index != NOT_FOUND;
}

//This function return the the index of the first empty frame in the page_table
int findEmptyFrame()
{
	int frame_index = NOT_FOUND;

	for (int i = 0; i < NUMBER_OF_FRAMES; i++)
		if (page_table[i] == NOT_FOUND)
		{
			frame_index = i;
			i = NUMBER_OF_FRAMES;    //to stop the loop
		}
	return frame_index;
}

void setCursor(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void execute(OPCODE opcode)
{
	switch (opcode){
	case MOV:
		AX_REGISTER = data_physical_memory[frame_index][offset];
		break;
	case PRN:
		setCursor(display_x, 6);
		std::cout << AX_REGISTER;

		print_page_table();

		display_x = display_x + 3;
	default:
		break;
	}
}

void print_page_table()
{
	int y = 8;
	for (int i = 0; i < NUMBER_OF_FRAMES; i++) {
		setCursor(display_x, y++);
		if (page_table[i] >= 0) {
			std::cout << page_table[i];
		}
	}
}