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

#include "simple-cache.h"

int scache_init(simple_cache* cache, int size, simple_cache_free_item_proc free_proc)
{
	if (!cache) return 0;
	cache->free_proc = free_proc;
	cache->size = size;
	cache->len = 0;
	cache->ids = (char**)malloc(sizeof(char*)*size);
	cache->values = (void**)malloc(sizeof(void*)*size);
	return (cache->ids && cache->values) ? 1 : 0;
}

void scache_free(simple_cache* cache)
{
	int i;
	if (!cache) return;

	if (cache->ids)
	{
		for (i=0; i<cache->len; i++)
			if (cache->ids[i]) free(cache->ids[i]);
		free(cache->ids);
		cache->ids = 0;
	}

	if (cache->values)
	{
		for (i=0; i<cache->len; i++)
			if (cache->values[i] && cache->free_proc) cache->free_proc(cache->values[i]);
		free(cache->values);
		cache->values = 0;
	}

	cache->size = 0;
	cache->len = 0;
}

void* scache_get(simple_cache* cache, const char* id, void* def)
{
	void* ret = def;
	int i;

	if (!cache || !cache->ids || !cache->values) return ret;

	for (i=0; i<cache->len; i++)
	{
		if (!strcmp(id, cache->ids[i]))
		{
			ret = cache->values[i];
			break;
		}
	}
	return ret;
}

int scache_set(simple_cache* cache, const char* id, void* val)
{
	int i;

	if (!cache || !cache->ids || !cache->values) return 0;

	for (i=0; i<cache->len; i++)
	{
		if (!strcmp(id, cache->ids[i])) break;
	}

	if (i >= cache->len)
	{
		if (i >= cache->size) return 0;
		cache->len++;
		cache->ids[i] = strdup(id);
		cache->values[i] = 0;
	}

	if (cache->values[i] && cache->free_proc) cache->free_proc(cache->values[i]);
	cache->values[i] = val;
	return 1;
}

