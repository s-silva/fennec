#include "../subtitles.h"


int open_subrip(string fname, struct sublist *list)
{
	struct subline  sub, *sub_mem = 0;
	sub_file        sfile;

	letter          line[single_line_length];
	string          p = 0, q = 0;
    int             a1, a2, a3, a4, b1, b2, b3, b4;
    int             len;

	sub_open(fname, &sfile);
    if(!sfile.data) return 0;

	list->count         = 0;
	list->current_index = 0;
	list->lines         = 0;

	for(;;)
	{
		for(;;)
		{
			if(!sub_read(&sfile, line)) return 0;

			if(swscanf(line, uni("%d:%d:%d,%d --> %d:%d:%d,%d"), &a1, &a2, &a3, &a4, &b1, &b2, &b3, &b4) < 8) continue;
			
			sub.starttime = (float)a1 * 3600000 + (float)a2 * 60000 + (float)a3 * 1000 + (float)a4;
			sub.endtime   = (float)b1 * 3600000 + (float)b2 * 60000 + (float)b3 * 1000 + (float)b4;
			sub.starttime /= 1000.0f;
			sub.endtime   /= 1000.0f;


			sub.text_mode = sub_text_mode_normal;
			sub.c_red     = 0xf;
			sub.c_green   = 0xf;
			sub.c_blue    = 0xf;

			if(!sub_read(&sfile, line)) return 0;

			p = q = line;

			for(sub.line_count = 1; sub.line_count < max_lines; sub.line_count++)
			{
				for (q=p,len=0; *p && *p!=uni('\r') && *p!=uni('\n') && *p!=uni('|') && str_ncmp(p,uni("[br]"),4); p++,len++);
				
				sub.lines[sub.line_count - 1] = malloc((len + 1) * sizeof(letter));

				if(!sub.lines[sub.line_count - 1]) return -1;

				str_ncpy(sub.lines[sub.line_count - 1], q, len);
				sub.lines[sub.line_count - 1][len] = uni('\0');

				if(!sub_read(&sfile, line)) return 0;
				p = q = line;

				if(!*p || *p == uni('\r') || *p == uni('\n')) break;
				//if(*p == '|') p++;
				//else while (*p++ != uni(']'));
			}
			break;
		}

		list->count++;
		
		if(sub_mem)
		{
			sub_mem = realloc(sub_mem, sizeof(struct subline) * list->count);
		}else{
			sub_mem = malloc(sizeof(struct subline) * list->count);
		}
		
		memcpy(&sub_mem[list->count - 1], &sub, sizeof(struct subline));
		list->lines = sub_mem;
	}

	sub_close(&sfile);
    return 1;
}