#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <time.h>

#define array_count(a) (sizeof(a)/sizeof((a)[0]))

/////////////////
//~ Naive Encoder

static void
base64_encode_naive(char *out, int out_len, char *in, int in_len) {
	static char *base64_encode_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	int in_index  = 0;
	int out_index = 0;
	
	while (in_index < (in_len - (in_len % 3))) {
		char a =   in[in_index+0]         >> 2;
		char b = ((in[in_index+0] & 0x03) << 4) | (in[in_index+1] >> 4);
		char c = ((in[in_index+1] & 0x0F) << 2) | (in[in_index+2] >> 6);
		char d =   in[in_index+2] & 0x3F;
		
		out[out_index+0] = base64_encode_table[a];
		out[out_index+1] = base64_encode_table[b];
		out[out_index+2] = base64_encode_table[c];
		out[out_index+3] = base64_encode_table[d];
		
		in_index  += 3;
		out_index += 4;
	}
	
	if (in_len % 3 == 1) {
		assert(in_index == in_len-1);
		
		char a =  in[in_index]         >> 2;
		char b = (in[in_index] & 0x03) << 4;
		
		out[out_index+0] = base64_encode_table[a];
		out[out_index+1] = base64_encode_table[b];
		out[out_index+2] = '=';
		out[out_index+3] = '=';
	} else if (in_len % 3 == 2) {
		assert(in_index == in_len-2);
		
		char a =   in[in_index+0]         >> 2;
		char b = ((in[in_index+0] & 0x03) << 4) | (in[in_index+1] >> 4);
		char c = ((in[in_index+1] & 0x0F) << 2);
		
		out[out_index+0] = base64_encode_table[a];
		out[out_index+1] = base64_encode_table[b];
		out[out_index+2] = base64_encode_table[c];
		out[out_index+3] = '=';
	}
}

/////////////////
//~ My Stupid Encoder
// This version is slower and a lot less readable but it was fun to implement.

#define USE_STATIC_BUFFER 1

static void
base64_encode_expand(char *out, int out_len, char *in, int in_len) {
	static char *base64_encode_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	
	int exp_len = in_len + (3 - (in_len % 3));
	
#if USE_STATIC_BUFFER
	short  exp[4*1024];
#else
	short *exp = malloc(exp_len * sizeof(short));
#endif
	
	{
		for (int i = 0; i < in_len; i += 1) {
			exp[i] = (short)in[i] << 8;
		}
		
		if (exp_len == in_len + 2) {
			exp[exp_len-2] = 64;
			exp[exp_len-1] = 64;
		} else if (exp_len == in_len + 1) {
			exp[exp_len-1] = 64;
		}
	}
	
	int in_index  = 0;
	int out_index = 0;
	
	while (in_index < exp_len) {
		char a = (char) ((exp[in_index+0]           >> 8) >> 2);
		char b = (char)(((exp[in_index+0] & 0x0300) >> 8) << 4) | (char)((exp[in_index+1] >> 8) >> 4) | (char)exp[in_index+0];
		char c = (char)(((exp[in_index+1] & 0x0F00) >> 8) << 2) | (char)((exp[in_index+2] >> 8) >> 6) | (char)exp[in_index+1];
		char d = (char) ((exp[in_index+2] & 0x3F00) >> 8)                                             | (char)exp[in_index+2];
		
		out[out_index+0] = base64_encode_table[a];
		out[out_index+1] = base64_encode_table[b];
		out[out_index+2] = base64_encode_table[c];
		out[out_index+3] = base64_encode_table[d];
		
		in_index  += 3;
		out_index += 4;
	}
	
#if USE_STATIC_BUFFER
#else
	free(exp); // Yay!
#endif
}

/////////////////
//~ Main

int main(void) {
	char *inputs[] = {
		"A",
		"AB",
		"ABC",
		"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incidunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrum exercitationem ullamco laboriosam, nisi ut aliquid ex ea commodi consequatur. Duis aute irure reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint obcaecat cupiditat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
	};
	
	char *expected[] = {
		"QQ==",
		"QUI=",
		"QUJD",
		"TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdCwgc2VkIGRvIGVpdXNtb2QgdGVtcG9yIGluY2lkdW50IHV0IGxhYm9yZSBldCBkb2xvcmUgbWFnbmEgYWxpcXVhLiBVdCBlbmltIGFkIG1pbmltIHZlbmlhbSwgcXVpcyBub3N0cnVtIGV4ZXJjaXRhdGlvbmVtIHVsbGFtY28gbGFib3Jpb3NhbSwgbmlzaSB1dCBhbGlxdWlkIGV4IGVhIGNvbW1vZGkgY29uc2VxdWF0dXIuIER1aXMgYXV0ZSBpcnVyZSByZXByZWhlbmRlcml0IGluIHZvbHVwdGF0ZSB2ZWxpdCBlc3NlIGNpbGx1bSBkb2xvcmUgZXUgZnVnaWF0IG51bGxhIHBhcmlhdHVyLiBFeGNlcHRldXIgc2ludCBvYmNhZWNhdCBjdXBpZGl0YXQgbm9uIHByb2lkZW50LCBzdW50IGluIGN1bHBhIHF1aSBvZmZpY2lhIGRlc2VydW50IG1vbGxpdCBhbmltIGlkIGVzdCBsYWJvcnVtLg=="
	};
	
	char out_buffer1[4096];
	char out_buffer2[4096];
	
	// Test correctness
	
	for (int i = 0; i < array_count(inputs); i += 1) {
		char *in = inputs[i];
		int in_len = strlen(in);
		
		int out_len = (in_len + 2) / 3 * 4;
		
		base64_encode_naive(out_buffer1, out_len, in, in_len);
		base64_encode_expand(out_buffer2, out_len, in, in_len);
		
		{
			int cmp = memcmp(out_buffer1, out_buffer2, out_len);
			assert(cmp == 0);
			
			assert(out_len == strlen(expected[i]));
			
			cmp = memcmp(out_buffer1, expected[i], out_len);
			assert(cmp == 0);
		}
	}
	
	// Test speed
	
#define TIMES 100000
	
	{
		char *in = inputs[3];
		int in_len = strlen(in);
		
		int out_len = (in_len + 2) / 3 * 4;
		
		// Warm up cache
		base64_encode_naive(out_buffer1, out_len, in, in_len);
		
		float start = (float)clock()/CLOCKS_PER_SEC;
		int c = 0;
		for (int i = 0; i < TIMES; i += 1) {
			base64_encode_naive(out_buffer1, out_len, in, in_len);
			c += out_buffer1[0]; // Make sure it doesn't get optimized away
		}
		float end = (float)clock()/CLOCKS_PER_SEC;
		float elapsed = end - start;
		
		printf("c = %d, elapsed = %f\n", c, elapsed);
	}
	
	{
		char *in = inputs[3];
		int in_len = strlen(in);
		
		int out_len = (in_len + 2) / 3 * 4;
		
		// Warm up cache
		base64_encode_expand(out_buffer1, out_len, in, in_len);
		
		float start = (float)clock()/CLOCKS_PER_SEC;
		int c = 0;
		for (int i = 0; i < TIMES; i += 1) {
			base64_encode_expand(out_buffer1, out_len, in, in_len);
			c += out_buffer1[0]; // Make sure it doesn't get optimized away
		}
		float end = (float)clock()/CLOCKS_PER_SEC;
		float elapsed = end - start;
		
		printf("c = %d, elapsed = %f\n", c, elapsed);
	}
	
	return 0;
}
