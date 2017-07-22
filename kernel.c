/*
 * kernel.c - the majority of our kernel, written in C
 *
 * Copyright 2017 Joey Pabalinas <alyptik@protonmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * GCC provides these header files automatically
 * They give us access to useful things like fixed-width types
 */
#if !defined(__cplusplus)
#	include <stdbool.h>
#endif
#include <stddef.h>
#include <stdint.h>

/* First, let's do some basic checks to make sure we are using our x86-elf cross-compiler correctly */
#if defined(__linux__)
/* #	error "This code must be compiled with a cross-compiler" */
#elif !defined(__i386__)
#	error "This code must be compiled with an x86-elf compiler"
#endif

/* This is the x86's VGA textmode buffer. To display text, we write data to this memory location */
volatile uint16_t *vga_buffer = (uint16_t *)0xB8000;
/* By default, the VGA textmode buffer has a size of 80x25 characters */
static const int VGA_COLS = 80;
static const int VGA_ROWS = 25;

/* We start displaying text in the top-left of the screen (column = 0, row = 0) */
static int term_col = 0;
static int term_row = 0;
/* Black background, White foreground */
static uint8_t term_color;

/* Hardware text mode color constants. */
enum vga_color {
	VGA_BLACK = 0,
	VGA_BLUE = 1,
	VGA_GREEN = 2,
	VGA_CYAN = 3,
	VGA_RED = 4,
	VGA_MAGENTA = 5,
	VGA_BROWN = 6,
	VGA_LIGHT_GREY = 7,
	VGA_DARK_GREY = 8,
	VGA_LIGHT_BLUE = 9,
	VGA_LIGHT_GREEN = 10,
	VGA_LIGHT_CYAN = 11,
	VGA_LIGHT_RED = 12,
	VGA_LIGHT_MAGENTA = 13,
	VGA_LIGHT_BROWN = 14,
	VGA_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | (bg << 4);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t)uc | ((uint16_t)color << 8);
}

static inline size_t strlen(const char *str)
{
	size_t len;
	for (len = 0; str[len]; len++);
	return len;
}

/* This function initiates the terminal by clearing it */
void term_init(void)
{
	term_color = vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK);
	/* Clear the textmode buffer */
	for (int col = 0; col < VGA_COLS; col++) {
		for (int row = 0; row < VGA_ROWS; row++) {
			/*
			 * The VGA textmode buffer has size (VGA_COLS * VGA_ROWS).
			 * Given this, we find an index into the buffer for our character
			 */
			const size_t index = (VGA_COLS * row) + col;
			/*
			 * Entries in the VGA buffer take the binary form BBBBFFFFCCCCCCCC, where:
			 * - B is the background color
			 * - F is the foreground color
			 * - C is the ASCII character
			 */
			/* Set the character to blank (a space character) */
			/* vga_buffer[index] = ' ' | ((uint16_t)term_color << 8); */
			vga_buffer[index] = vga_entry(' ', term_color);
		}
	}
}

/* This function places a single character onto the screen */
void term_putc(char c)
{
	/* Remember - we don't want to display ALL characters! */
	switch (c) {
	/* Newline characters should return the column to 0, and increment the row */
	case '\n': {
			term_col = 0;
			term_row++;
			break;
		}

	/* Normal characters just get displayed and then increment the column */
	default: {
			/* Like before, calculate the buffer index */
			const size_t index = (VGA_COLS * term_row) + term_col;
			vga_buffer[index] = ((uint16_t)term_color << 8) | c;
			term_col++;
		}
	}

	/*
	 * What happens if we get past the last column? We need to reset the
	 * column to 0, and increment the row to get to a new line
	 */
	if (term_col >= VGA_COLS) {
		term_col = 0;
		term_row++;
	}

	/*
	 * What happens if we get past the last row? We need to reset both
	 * column and row to 0 in order to loop back to the top of the screen
	 */
	if (term_row >= VGA_ROWS) {
		term_col = 0;
		term_row = 0;
	}
}

/* This function prints an entire string onto the screen */
void term_print(const char *str)
{
	/* Keep placing characters until we hit the null-terminating character ('\0') */
	for (size_t i = 0; i < strlen(str); i++)
		term_putc(str[i]);
}

/* This is our kernel's main function */
void kernel_main(void)
{
	/* We're here! Let's initiate the terminal and display a message to show we got here. */

	/* Initiate terminal */
	term_init();

	/* Display some messages */
	term_print("Hello, World!\n");
	term_print("Welcome to the kernel.\n");
}
