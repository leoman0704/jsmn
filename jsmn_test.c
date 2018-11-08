#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn.h"

#define MAX_JSON_TOKEN_EXPECTED 128

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
	jsmntok_t *result = NULL;

	int i = 0;

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
int parseStringValue(char *buf, const char *jsonString, jsmntok_t *token)
{
	int size = 0;
	if (token->type != JSMN_STRING)
	{
		//printf("Token was not a string.\r\n");
		return -1;
	}
	size = (int)(token->end - token->start);
	memcpy(buf, jsonString + token->start, size);
	buf[size] = '\0';
	return 0;
}

void jf_free_array(jf_array *pArray)
{
	if (!pArray)
		return;
	if (*pArray->token)
		free(pArray->token);
	free(pArray);
}

jf_array *findTokenArray(const int tokenCount, const char *key, const char *jsonpayload, jsmntok_t *token)
{
	jsmntok_t *ftok = NULL;
	jf_array *array = malloc(sizeof(jf_array));
	if (!array)
		return NULL;

	array->cnt = 0;
	array->token = NULL;

	ftok = findToken(key, jsonpayload, token);
	if (ftok)
	{

		array->cnt = ftok->size;
		array->token = calloc(1, sizeof(jsmntok_t *) * array->cnt);
		if (!array->token)
		{
			free(array);
			return NULL;
		}

		int token_found = 0;
		for (int i = 0; i <= tokenCount; i++)
		{
			jsmntok_t *tok_next = NULL;
			tok_next = ftok + i;

			if (tok_next && tok_next->type)
			{
				if (tok_next->type == JSMN_OBJECT)
				{
					array->token[token_found] = tok_next;

					token_found++;

					if (token_found > array->cnt)
					{
						break;
					}
				}
			}
		}
	}

	return array;
}
int findValuebyArrayKey(char *buf, char *key, char *jsonpayload, jsmntok_t *token)
{
	char buffer[64] = {0};
	jsmntok_t *token_next = NULL;

	if (token->type == JSMN_STRING)
	{
		if (parseStringValue(buffer, jsonpayload, token++) >= 0)
		{

			if (strcmp(buffer, key) == 0)
			{
				token_next = token++;
				if (parseStringValue(buf, jsonpayload, token_next) >= 0)
				{
					//printf("buf=%s\r\n", buf);
					return 0;
				}
			}
		}
	}
	return -1;
}
int testArrayParser(char *jsonpayload)
{
	jsmn_parser jsonParser;
	int tokenCount = 0;
	int payloadLen = 0;
	jsmntok_t jsonTokenStruct[MAX_JSON_TOKEN_EXPECTED];
	jf_array *array = NULL;
	memset(&jsonParser, 0, sizeof(jsonParser));
	jsmn_init(&jsonParser);

	payloadLen = strlen(jsonpayload);
	tokenCount = jsmn_parse(&jsonParser, jsonpayload, (int)payloadLen, jsonTokenStruct, MAX_JSON_TOKEN_EXPECTED);

	array = findTokenArray(tokenCount, "act", jsonpayload, jsonTokenStruct);
	if (array)
	{
		for (int j = 0; j < array->cnt; j++)
		{
			jsmntok_t *token = findToken("air", jsonpayload, array->token[j]);
			if (token)
			{
				printf("air=%.*s \n", token->end - token->start, jsonpayload + token->start);
			}
		}
	}
	if (array)
		jf_free_array(array);
	return 0;
}
int testArrayParser_type1(char *jsonpayload)
{
	jsmn_parser jsonParser;
	int tokenCount = 0;
	int payloadLen = strlen(jsonpayload);
	char buffer[64] = {0};
	jsmntok_t jsonTokenStruct[MAX_JSON_TOKEN_EXPECTED];
	jsmn_init(&jsonParser);

	tokenCount = jsmn_parse(&jsonParser, jsonpayload, (int)payloadLen, jsonTokenStruct, MAX_JSON_TOKEN_EXPECTED);
	for (int j = 0; j < tokenCount; j++)
	{
		jsmntok_t *token = &jsonTokenStruct[j];
		if (findValuebyArrayKey(buffer, "air", jsonpayload, token) == 0)
			printf("air = %s\r\n", buffer);
	}

	return 0;
}

int main()
{

	char jsonpayload[] = "{\"command\":[{\"a1\":\"b1\",\"a2\":\"b2\"},{\"action\":\"1\",\"value\":\"2\"}],\"act\":[{\"air\":\"1\",\"value\":\"a\"},{\"air\":\"b\",\"qos\":\"c\"}]}";
	testArrayParser(jsonpayload);

	char jsonpayload2[] = "[{\"air\":\"1\",\"value\":\"a\"},{\"air\":\"b\",\"qos\":\"c\"}]";
	testArrayParser_type1(jsonpayload2);

	return 0;
}
