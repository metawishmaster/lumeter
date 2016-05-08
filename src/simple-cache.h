/*
 * Copyright (c) 2009 Tarannko Oleg <oleg@taranenko.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef void (*simple_cache_free_item_proc)(void* item);
typedef struct simple_cache_tag
{
	simple_cache_free_item_proc free_proc;
	int size;
	int len;
	char** ids;
	void** values;
} simple_cache;

int scache_init(simple_cache* cache, int size, simple_cache_free_item_proc free_proc);
void scache_free(simple_cache* cache);
void* scache_get(simple_cache* cache, const char* id, void* def);
int scache_set(simple_cache* cache, const char* id, void* val);
