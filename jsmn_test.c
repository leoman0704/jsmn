#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn.h"
#define MAX_JSON_TOKEN_EXPECTED 128

#define uint32_t unsigned long
#define int32_t long

typedef struct _jf_array
{
	int cnt;
	jsmntok_t **token;
} jf_array;

int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
	if (tok->type == JSMN_STRING)
	{
		if ((int)strlen(s) == tok->end - tok->start)
		{
			if (strncmp(json + tok->start, s, (size_t)(tok->end - tok->start)) == 0)
			{
				return 0;
			}
		}
	}
	return -1;
}

jsmntok_t *findToken(const char *key, const char *jsonString, jsmntok_t *token)
{
	jsmntok_t *result = token;
	int i;

	if (token->type != JSMN_OBJECT)
	{
		printf("Token was not an object.\r\n");
		return NULL;
	}

	if (token->size == 0)
	{
		return NULL;
	}

	result = token + 1;

	for (i = 0; i < token->size; i++)
	{
		if (0 == jsoneq(jsonString, result, key))
		{
			return result + 1;
		}

		int propertyEnd = (result + 1)->end;
		result += 2;

		if (result->start == 0)
		{
			return NULL;
		}
		while (result->start < propertyEnd)
			result++;
	}

	return NULL;
}

void jf_free_array(jf_array *pArray)
{
	if (!pArray)
		return;
	if (pArray->token)
		free(pArray->token);
	free(pArray);
}

jf_array *findTokenArray(const int tokenCount, const char *key, const char *jsonpayload, jsmntok_t *token)
{
	jf_array *array = malloc(sizeof(jf_array));
	if (!array)
		return NULL;

	array->cnt = 0;
	array->token = NULL;

	jsmntok_t *ftok = findToken(key, jsonpayload, token);
	if (ftok)
	{
		//printf("%s: %.*s size=%d\n", key, ftok->end - ftok->start, jsonpayload + ftok->start, ftok->size);
		array->cnt = ftok->size;
		array->token = malloc(sizeof(jsmntok_t *) * array->cnt);
		memset(array->token, 0, sizeof(jsmntok_t *) * array->cnt);
		int token_found = 0;
		for (int i = 0; i <= tokenCount; i++)
		{
			jsmntok_t *tok_next = ftok + i;

			if (tok_next->type == JSMN_OBJECT)
			{
				array->token[token_found] = tok_next;
				printf(" %.*s size=%d type=%d\n", tok_next->end - tok_next->start, jsonpayload + tok_next->start, tok_next->size, tok_next->type);
				//printf("token_next= %p\r\n", tok_next);
				//printf("token_found=%d\r\n", token_found);
				token_found++;

				if (token_found > array->cnt)
				{
					printf("token_found=%d\r\n", token_found);
					break;
				}
			}
		}
	}

	return array;
}

int main()
{
	jsmn_parser jsonParser;
	static jsmntok_t jsonTokenStruct[MAX_JSON_TOKEN_EXPECTED];

	//    jsmntok_t *aJsonHandler = NULL;

	int32_t tokenCount;

	char jsonpayload[] = "{\"command\":[{\"a1\":\"b1\",\"a2\":\"b2\"},{\"action\":\"1\",\"value\":\"2\"}],\"act\":[{\"airb\":\"1\",\"value\":\"a\"},{\"air\":\"b\",\"qos\":\"c\"}]}";

	int payloadLen = strlen(jsonpayload);

	jsmn_init(&jsonParser);

	tokenCount = jsmn_parse(&jsonParser, jsonpayload, (int)payloadLen, jsonTokenStruct, MAX_JSON_TOKEN_EXPECTED);

	printf("tokenCount=%ld\n", tokenCount);
	jf_array *array = findTokenArray(tokenCount, "act", jsonpayload, jsonTokenStruct);

	for (int j = 0; j < array->cnt; j++)
	{
		printf("j =%d ,array->cnt=%d array ptr=%p\r\n", j, array->cnt, array->token[j]);
		jsmntok_t *token = findToken("air", jsonpayload, array->token[j]);
		if (token)
		{
			//printf("found air token\r\n");
			printf("air=%.*s \n", token->end - token->start, jsonpayload + token->start);
		}
	}

	if (array)
		jf_free_array(array);
	return 0;
}
