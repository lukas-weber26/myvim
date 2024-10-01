#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

typedef struct line {
	char * line_text;
	int line_length;
} line;

typedef struct file {
	char * file_name;
	FILE * file_pointer;
	line ** lines;
	int max_lines;
	int line_count;
	int x_pos;
	int y_pos;
} file;

line * line_create_empty() {
	line * empty_line = calloc(1, sizeof(line));	
	empty_line->line_length = 2;
	empty_line->line_text =	strdup(" \n");	
	return empty_line;
}

line * line_create_string(char * string, int length) {
	line * empty_line = calloc(1, sizeof(line));	
	empty_line->line_length = length;
	empty_line->line_text =	strdup(string);	
	return empty_line;
}

void file_open(file * file){
	file->file_pointer = fopen(file->file_name,"r");
	if (file->file_pointer != NULL){
		//set up the file by reading
		file->line_count = 0;
		file->max_lines= 10;
		file->lines = calloc(10, sizeof(line));
		int c;
		size_t buffer_size = 512;
		char * line_buffer = calloc(buffer_size,sizeof(char));

		while((c = getline(&line_buffer, &buffer_size,file->file_pointer)) != -1){
			file->lines[file->line_count] = line_create_string(line_buffer,c);	
			file->line_count += 1;

			if (file->line_count == file->max_lines){
				file->max_lines *= 2;
				file->lines = realloc(file->lines, sizeof(line)*file->max_lines);
			}
		}		
		
		fclose(file->file_pointer);		
		
	} else {
		//set up an empty file	
		file->line_count = 1;
		file->max_lines= 10;
		file->lines = calloc(10, sizeof(line));
		file->lines[0] = line_create_empty();
	}

	//get rid of information that can no longer be used.
	file->file_pointer=NULL;
	return;
}

void line_render(line * line, int i){
	mvprintw(i,0,"%d:",i);
	mvprintw(i,3,"%s",line->line_text);
}

void file_render(file * file){
	int x,y;
	y = getmaxy(stdscr);	
	x = getmaxx(stdscr);	

	mvprintw(0, (x-strlen(file->file_name))/2, "%s.",file->file_name);
		
	for (int i = 0; i<file->line_count && i<x-1; i++){
		line_render(file->lines[i], i+1);	
	}	
}

void line_free(line * line){
	free(line->line_text);
	free(line);
}

void file_free(file * file){
	for (int i = 0; i<file->line_count; i++){
		line_free(file->lines[i]);	
	}	
	free(file->lines);
	free(file);
}

void move_cursor(file * file, int dx, int dy) {
	file->y_pos+=dy;
	file->x_pos+=dx;
	
	if (file->y_pos < 1){
		file->y_pos = 1;
	}
	
	if (file->x_pos < 3){
		file->x_pos = 3;
	}

	move(file->y_pos, file->x_pos);
	
	refresh();
}

int main(int argc, char * argv[]){	
	if (argc != 2){
		printf("Usage: myvim <filename>\n");		
		exit(0);
	}

	file * memory_file = calloc(1,sizeof(file)); 
	memory_file->file_name = argv[1];	

	file_open(memory_file);

	initscr();
	nonl();
	cbreak();
	noecho();

	if (has_colors()){
		start_color();
	}

	file_render(memory_file);
	memory_file->x_pos = 3;
	memory_file->y_pos = 1;
	move(memory_file->x_pos,memory_file->y_pos);
	refresh();	

	//loop here or something?
	int input;

	while((input = getch())){
		//figure out what to do	
		switch(input){
			case 'h': move_cursor(memory_file, -1, 0); break;
			case 'j': move_cursor(memory_file, 0, 1); break;
			case 'k': move_cursor(memory_file, 0, -1); break;
			case 'l': move_cursor(memory_file, 1, 0); break;
			default: break;	
		}
	}

	refresh();
	endwin();
	file_free(memory_file);
}
