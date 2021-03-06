/*************************************************************************
$Id: lang.c 151 2005-08-10 01:10:11Z terminal $

Bersirc - A cross platform IRC client utilizing the Claro GUI Toolkit.
Copyright (C) 2004-2005 Theo P. Julienne and Nicholas S. Copeland

This application is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This application is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this application; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**************************************************************************/

#include "includes.h"

char *lang_phrase = 0;
XMLFile *lang_xml;
XMLFile *lang_xml_fback;
char lang_tmp_buf[1024];

void lang_init( char *lang )
{
	char filename[64];
	char *fullname;
	
	sprintf( filename, "lang-%s.xml", lang );
	fullname = config_file_loc( filename );
	
	lang_xml = c_xml_create( );
	c_xml_load_file( lang_xml, fullname );
	
	if ( !strcmp( lang, LANG_FALLBACK ) )
	{
		lang_xml_fback = lang_xml;
	}
	else
	{
		sprintf( filename, "lang-%s.xml", LANG_FALLBACK );
		fullname = config_file_loc( filename );
		
		lang_xml_fback = c_xml_create( );
		c_xml_load_file( lang_xml_fback, fullname );
	}
	
	if ( lang_xml_fback == 0 )
	{
		printf( "Warning: Language fallback does not exist!\n" );
	}
}

char *lang_phrase_quick( char *phrase )
{
	XMLItem *item;
	char path[128];
	char *val;
	
	sprintf( path, "textblocks.%s", phrase );
	
	item = c_xml_get_path( lang_xml, path );
	
	if ( item == 0 )
		item = c_xml_get_path( lang_xml_fback, path );
	
	if ( item == 0 )
	{
		return "";
	}
	
	val = c_xml_attrib_get( item, "text" );
	
	return val;
}

int lang_str_overlap( char *a, char *b );

/* First arguments: out buffer, out size, phrase
 * Then: name, value, ...
 * Finally: 0
 *
 * Examples:
 * lang_phrase_parse( &buf, 1024, "blah", "name1", "value1", 0 );
 * lang_phrase_parse( &buf, 1024, "blah", "name1", "value1", "name2", "value2", 0 );
 */
// FIXME: I don't trust this totally. More testing needed. Seems to crash in some cases.
//        Find out where, why, then fix. :)
char *lang_phrase_parse( char *buf, int size, char *phrase, ... )
{
	char *text, *rem;
	char *p_name, *p_value;
	int ns, vs;
	char *outtxt = NULL, *newtxt = NULL;
	int outsz = 0, oldsz = 0;
	va_list ap;
	int a, b, c;
	
	text = lang_phrase_quick( phrase );
	
	if ( text == NULL )
		return ""; // oof
	
	va_start( ap, phrase );
	
	outsz = strlen( text ) + 1;
	outtxt = (char *)malloc( outsz );
	
	strcpy( outtxt, text );
	
	while ( ( p_name = va_arg( ap, char * ) ) != NULL )
	{
		/* value comes after name, get it as well */
		p_value = va_arg( ap, char * );
		
		if ( p_name == NULL || p_value == NULL )
			continue;
		
		ns = strlen( p_name );
		vs = strlen( p_value );
		
		for ( a = 0; outtxt[a] != 0; a++ )
		{
			if ( outtxt[a] != '$' )
				continue; /* NOT a var */
			
			b = lang_str_overlap( outtxt+a+1, p_name );
			
			if ( b != ns )
				continue; /* NOT a match */
			
			/* difference between "$name" and "value" */
			c = vs - (ns + 1);
			
			/* save current size */
			oldsz = outsz;
			
			/* add the difference to the size of the memory block */
			outsz += c;
			
			/* allocate our new block */
			newtxt = (char *)malloc( outsz );
			
			/* copy over the initial bytes of the string from the original */
			if ( a > 0 )
				memcpy( newtxt, outtxt, a );
			
			/* copy over the value string */
			memcpy( newtxt+a, p_value, vs );
			
			/* copy over the remaining contents of the string, including \0 */
			rem = outtxt + a + 1 + ns; /* <string> + <offset> + $ + <name> */
			memcpy( newtxt+a+vs, rem, strlen( rem ) + 1 );
			
			/* kill the old text buffer, replace with the new one */
			free( outtxt );
			outtxt = newtxt;
			
			/* reset to the start of checking */
			a = 0;
		}
	}
	
	va_end( ap );
	
	memset( buf, 0, size );
	strncpy( buf, outtxt, size-1 );
	free( outtxt );
	
	return text;
}

// returns the number of chars at the start of a and b that match
int lang_str_overlap( char *a, char *b )
{
	int c = 0;
	
	while ( *(a+c) && *(b+c) && *(a+c) == *(b+c) )
		c++;
	
	return c;
}

//
