
/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:cmdui.c                                                             *
* Description: hardware init relative routines                             *
* Author: Ted Hsu 			                                               *
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "SoFlexible.h"
#include "command.h"

#define COMMAND_TABLE_SIZE		40

/* =================================================== */
#define COMMAND_HISTORY_COUNT	22
INT32 CmdHistoryHeadIndex = 0;
INT32 CmdHistoryCount = 0;
INT32 CmdHistoryTailIndex = 0;
INT8 CmdHistroy[COMMAND_HISTORY_COUNT][MAX_COMMAND_LEN];

#define NEXT_CMD_INDEX(x)			{x++;if(x>=COMMAND_HISTORY_COUNT) x=0;}
#define PREVIOUS_CMD_INDEX(x)		{x--;if(x<0) x=(COMMAND_HISTORY_COUNT-1);}
#define NEW_CMD_INDEX(x)			{x+=CmdHistoryHeadIndex;x=x%COMMAND_HISTORY_COUNT;}

/* =================================================== */
UserSymbolStruct UserSymbolTable[MAX_USER_SYMBOL_SIZE];
UINT32	DefaultUserSymbolNum = 0;


/* =================================================== */
INT8 DefaultStrDelimit[] = {' ', 0x0A, 0x0D, '\0'}; 



/* =================================================== */
CommandTableStruct_T fLib_default_CmdTable[COMMAND_TABLE_SIZE] =
{
	{{"", NULL,}, "USER_SYMBOL [operator operand]/[-x]\nAccess an user-defined symbol.\n",
		0, 0, 2, 0, 1, 0, 
		UserSymbolCmd, 0,  0,
		{
			{IS_ARITHMETIC_OP_STR, NULL, 0, FALSE, "operator - Could be =, +=, -=, /=, %=.\n"}, 
			{IS_NUM|IS_REG|IS_MEM|IS_USER_SYMBOL, NULL, 0, FALSE, "operand  - Could be constant or user-defined symbol.\n"},
		},
		{
			{"-x", IS_NULL, {NULL,}, NULL, 0, FALSE, "Display hexadecimal integer.\n"},
		},
	},
	{{"symbol", "sym", NULL}, "symbol/sym [symbol_name] [-d] [-x]\nCreate/Delete an user-defined symbol.\n",
		0, 0, 1, 0, 2, 0, 
		SymbolCmd, 0, 0,
		{
			{IS_STR, NULL, 0, FALSE, "symbol_name - The symbol name to be created/deleted.\n"},
		},
		{
			{"-d", IS_NULL, {NULL,}, NULL, 0, FALSE, "To delete the symbol\n"}, 
			{"-x", IS_NULL, {NULL,}, NULL, 0, FALSE, "Display hexadecimal integer.\n"},
		},
	},

	{{"help", "?", "h"}, "help [command]\n",
		0, 0, 1, 0, 0, 0, 
		HelpCmd, 0, 0,
		{
			{IS_STR, NULL, 0, FALSE, "command - Give help for specified command.\n"},
		},
	},	
	{{"if", NULL,}, "if operand1 operator operand2\n", 
		0, 0, 3, 3, 0, 0, 
		IfCmd, 0, 0,
		{
			{IS_NUM|IS_REG|IS_MEM|IS_USER_SYMBOL, NULL, 0, FALSE, "operand1 - Could be constant or user-defined symbol.\n"}, 
			{IS_LOGICAL_OP_STR, NULL, 0, FALSE, "operand2 - Could be constant or user-defined symbol.\n"}, 
			{IS_NUM|IS_REG|IS_MEM|IS_USER_SYMBOL, NULL, 0, FALSE, "operator - Could be >, >=, ==, <=, <, !=.\n"},
		},
	},
	{{"while", NULL,}, "while operand1 operator operand2\n", 
		0, 0, 3, 3, 0, 0, 
		WhileCmd, 0, 0,
		{
			{IS_NUM|IS_REG|IS_MEM|IS_USER_SYMBOL, NULL, 0, FALSE, "operand1 - Could be constant or user-defined symbol.\n"}, 
			{IS_LOGICAL_OP_STR, NULL, 0, FALSE, "operand2 - Could be constant or user-defined symbol.\n"}, 
			{IS_NUM|IS_REG|IS_MEM|IS_USER_SYMBOL, NULL, 0, FALSE, "operator - Could be >, >=, ==, <=, <, !=.\n"},
		},
	},
	/* end */
	{NULL,
	},
};

/* =================================================== */
CommandTableStruct_T fLib_CmdTable[COMMAND_TABLE_SIZE+1] =
{
#if 0	
	{{"", NULL,}, "USER_SYMBOL [operator operand]/[-x]\nAccess an user-defined symbol.\n",
		0, 0, 2, 0, 1, 0, 
		UserSymbolCmd, 0,  0,
		{
			{IS_ARITHMETIC_OP_STR, NULL, 0, FALSE, "operator - Could be =, +=, -=, /=, %=.\n"}, 
			{IS_NUM|IS_REG|IS_MEM|IS_USER_SYMBOL, NULL, 0, FALSE, "operand  - Could be constant or user-defined symbol.\n"},
		},
		{
			{"-x", IS_NULL, {NULL,}, NULL, 0, FALSE, "Display hexadecimal integer.\n"},
		},
	},
	{{"symbol", "sym", NULL}, "symbol/sym [symbol_name] [-d] [-x]\nCreate/Delete an user-defined symbol.\n",
		0, 0, 1, 0, 2, 0, 
		SymbolCmd, 0, 0,
		{
			{IS_STR, NULL, 0, FALSE, "symbol_name - The symbol name to be created/deleted.\n"},
		},
		{
			{"-d", IS_NULL, {NULL,}, NULL, 0, FALSE, "To delete the symbol\n"}, 
			{"-x", IS_NULL, {NULL,}, NULL, 0, FALSE, "Display hexadecimal integer.\n"},
		},
	},

	{{"help", "?", "h"}, "help [command]\n",
		0, 0, 1, 0, 0, 0, 
		HelpCmd, 0, 0,
		{
			{IS_STR, NULL, 0, FALSE, "command - Give help for specified command.\n"},
		},
	},	
	{{"if", NULL,}, "if operand1 operator operand2\n", 
		0, 0, 3, 3, 0, 0, 
		IfCmd, 0, 0,
		{
			{IS_NUM|IS_REG|IS_MEM|IS_USER_SYMBOL, NULL, 0, FALSE, "operand1 - Could be constant or user-defined symbol.\n"}, 
			{IS_LOGICAL_OP_STR, NULL, 0, FALSE, "operand2 - Could be constant or user-defined symbol.\n"}, 
			{IS_NUM|IS_REG|IS_MEM|IS_USER_SYMBOL, NULL, 0, FALSE, "operator - Could be >, >=, ==, <=, <, !=.\n"},
		},
	},
	{{"while", NULL,}, "while operand1 operator operand2\n", 
		0, 0, 3, 3, 0, 0, 
		WhileCmd, 0, 0,
		{
			{IS_NUM|IS_REG|IS_MEM|IS_USER_SYMBOL, NULL, 0, FALSE, "operand1 - Could be constant or user-defined symbol.\n"}, 
			{IS_LOGICAL_OP_STR, NULL, 0, FALSE, "operand2 - Could be constant or user-defined symbol.\n"}, 
			{IS_NUM|IS_REG|IS_MEM|IS_USER_SYMBOL, NULL, 0, FALSE, "operator - Could be >, >=, ==, <=, <, !=.\n"},
		},
	},

	/* CLEAR_COMMAND */
	{{"clear", "clr", NULL}, "clear\nClear debug window.\n",
		0, 0, 0, 0, 0, 0, 
		ClearCmd, 0, 0, 
	},
#endif	
	/* end */
	{NULL,
	},
};


/************************************************************************************************/
/************************************************************************************************/
UINT32 fLib_InsertCommand(CommandTableStruct_T CmdTable[])
{
	UINT32	i, j;
	
//ycmo : init CmdTable
	memset(fLib_CmdTable,0x0 , sizeof(CommandTableStruct_T)* COMMAND_TABLE_SIZE);
	for(i = 0; i < COMMAND_TABLE_SIZE; i++)
	{
		if(fLib_default_CmdTable[i].Name[0] == NULL)
			break;
		memcpy(&fLib_CmdTable[i], &fLib_default_CmdTable[i], sizeof(CommandTableStruct_T));
	}

	/* starting to compare with the command in CmdTable */
	for(i = 0, j = 0; i < COMMAND_TABLE_SIZE; i++)
	{
		if(CmdTable[j].Name[0] == NULL)
			return TRUE;
			
		if(fLib_CmdTable[i].Name[0] == NULL)
		{
			memcpy(&fLib_CmdTable[i], &CmdTable[j], sizeof(CommandTableStruct_T));
			j++;
			fLib_CmdTable[i+1].Name[0] = NULL;
		}
	}
	return FALSE;
}

void fLib_InitUserSymbol(UserSymbolStruct symbol[])
{
	UINT32	i;

	for(i = 0; i < MAX_USER_SYMBOL_SIZE; i++)
		UserSymbolTable[i].IsFree = TRUE;
		
	if(symbol != NULL)
	{
		i = 0;
		while(strlen(symbol[i].SymName) > 0)
		{
			NewUserSymbol(symbol[i].SymName);
			WriteUserSymbol(i, symbol[i].SymValue);
			DefaultUserSymbolNum++;
			i++;
		}
	}
}

UINT32 fLib_ExecuteCommand(INT8 *CommandStr)
{
	UINT32	CommandNum, CmdIndex, CmdResult, i;
	INT8	*CmdList[MAX_COMMAND_NUM], DupCommandStr[MAX_COMMAND_NUM], *cmd, TempStr[MAX_COMMAND_LEN + 3];

	if(CommandStr[0] == 0)
		return FALSE;
		
	/* retrieve all commands to CmdList */
	CommandNum = 0;
	strcpy(DupCommandStr, CommandStr);
	cmd = strtok(DupCommandStr, ";");
	do
	{
		if(strlen(cmd) > 0)
		{
			CmdList[CommandNum] = cmd;
			CommandNum++;
		}
	}while((cmd = strtok(NULL, ";")) != NULL);

	/* execute all commands */
	for(i = 0; i < CommandNum; i++)
	{
		/* print this command */
		printf("<command>: %s\n", CmdList[i]);
		
		/* copy command to TempStr */
		strcpy(TempStr ,CmdList[i]);
		
		/* parse this command */
		if((CmdIndex = ParseCommand(TempStr, fLib_CmdTable)) == -1)
		{
			printf("<Unrecognised command>: %s\n", TempStr);
			break;
		}
		/* free the previous argumant and option settings */
		FreeCmdArgumentOption(CmdIndex, fLib_CmdTable);

		/* parse argument and option */
		if(!ParseCommandParameter(TempStr, CmdIndex, fLib_CmdTable))
		{
			printf(fLib_CmdTable[CmdIndex].HelpStr);
			break;
		}

		/* execute command */
		CmdResult = fLib_CmdTable[CmdIndex].pf(&fLib_CmdTable[CmdIndex]);

		switch(CmdResult)
		{
			case WHILE_COMMAND:
				/* let i = -1, will execute command from the 1st command */
				i = -1;
				break;
			case NEED_HELP_COMMAND:
				/* Print help string */
				printf(fLib_CmdTable[CmdIndex].HelpStr);
				return FALSE;
			case IF_ERROR_COMMAND:
				/* if the "if" command returns error, finishs the commands execution */
				/* but returns TRUE for normal termination. */
				return TRUE;
			case ERROR_COMMAND:
				/* end this command line */
				return FALSE;
			case OK_COMMAND:  // added by silas
			default:
			    return CmdResult;  // added by silas for findout status
				break;
		}
#if 1	
		if(IsCommandBreak())
		{
			printf("User break!!!!!\n");
			break;
		}
#endif		
	}

	return TRUE;
}


/////////////////////////////////////////////////////
//
//	Only for detail function call subroutine
//
/////////////////////////////////////////////////////


UINT32 ParseCommand(INT8 *cmd, CommandTableStruct_T CmdTable[])
{
	INT8	*str, TempStr[MAX_COMMAND_LEN];
	UINT32	i, j;
	UINT32	SymbolIndex;
	
	/* get command name */
	strcpy(TempStr, cmd);
	str = strtok(TempStr, " ;\0");

	/* starting to compare with the command in CmdTable */
	i = 0;
	while(CmdTable[i].Name[0] != NULL)
	{
		j = 0;
		do
		{
			if(!strcmp(str, CmdTable[i].Name[j]))
			{
				return i;
			}
			j++;
		}while((CmdTable[i].Name[j] != NULL) && (j < COMMAND_NAME_COUNT));
		i++;
	}

	/* if it is user-defined symbol */
	if(ParameterIsUserSymbol(str, &SymbolIndex, NULL))
	{
		/* set SymbolIndex to Key elemant of MEM_COMMAND */
		CmdTable[USER_SYMBOL_COMMAND].Key = SymbolIndex;
		return USER_SYMBOL_COMMAND;
	}
	
	return -1;
}


BOOL IsCommandBreak(void)
{
	INT8 ch;

	if(ch == ESC)
		return TRUE;

	return FALSE;
}


/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
void SaveCmdToHistory(INT8 *cmd)
{
	if(strlen(cmd) == 0)
		return;

	strcpy(CmdHistroy[CmdHistoryTailIndex], cmd);
	NEXT_CMD_INDEX(CmdHistoryTailIndex)

	if(CmdHistoryHeadIndex == CmdHistoryTailIndex)
		NEXT_CMD_INDEX(CmdHistoryHeadIndex)
	else
		CmdHistoryCount++;
	return;
}

/************************************************************************************************/
/************************************************************************************************/
INT8 *GetCmdFromHistory(UINT32 offset)
{
	if(CmdHistoryCount == 0)
		return NULL;
		
	if(offset >= CmdHistoryCount)
		return NULL;

	/* let the offset is to the real location in CmdHistroy */
	NEW_CMD_INDEX(offset)
	return CmdHistroy[offset];
}

/************************************************************************************************/
/************************************************************************************************/
void RemoveCurrCmdFromHistory()
{
	if(CmdHistoryCount > 0)
	{
		CmdHistoryCount--;
		PREVIOUS_CMD_INDEX(CmdHistoryTailIndex);
	}
}

/************************************************************************************************/
/************************************************************************************************/
void ClearCmdHistory()
{
	CmdHistoryHeadIndex = 0;
	CmdHistoryCount = 0;
	CmdHistoryTailIndex = 0;
}

/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
UINT32 NewUserSymbol(INT8 *str)
{
	UINT32	i;

	/* find an free location */
	for(i = 0; i < MAX_USER_SYMBOL_SIZE; i++)
	{
		if(UserSymbolTable[i].IsFree == TRUE)
		{
			strcpy(UserSymbolTable[i].SymName, str);
			UserSymbolTable[i].SymValue = 0;
			UserSymbolTable[i].IsFree = FALSE;
			return TRUE;
		}
		
	}
	return FALSE;
}

/************************************************************************************************/
/************************************************************************************************/
UINT32 DeleteUserSymbol(UINT32 SymIndex)
{
	/* if deleted item is in Default User Symbol */
	if(SymIndex < DefaultUserSymbolNum)
		return FALSE;

	if(SymIndex >= MAX_USER_SYMBOL_SIZE)
		return FALSE;
		
	UserSymbolTable[SymIndex].IsFree = TRUE;
	
	return TRUE;
}

/************************************************************************************************/
/************************************************************************************************/
INT8 *ReadUserSymbol(UINT32 SymIndex, UINT32 *SymValue)
{
	*SymValue = 0;
	if(SymIndex >= MAX_USER_SYMBOL_SIZE)
		return NULL;
	if(UserSymbolTable[SymIndex].IsFree == TRUE)
		return NULL;
		
	*SymValue = UserSymbolTable[SymIndex].SymValue;
	return UserSymbolTable[SymIndex].SymName;
}

/************************************************************************************************/
/************************************************************************************************/
void WriteUserSymbol(UINT32 SymIndex, UINT32 NewValue)
{
	if(SymIndex >= MAX_USER_SYMBOL_SIZE)
		return;
	if(UserSymbolTable[SymIndex].IsFree == TRUE)
		return;

	UserSymbolTable[SymIndex].SymValue = NewValue;
}

/************************************************************************************************/
/************************************************************************************************/
void DeleteAllUserSymbol()
{
	UINT32	i;

	for(i = 0; i < MAX_USER_SYMBOL_SIZE; i++)
		UserSymbolTable[i].IsFree = FALSE;
}

/************************************************************************************************/
/************************************************************************************************/
UINT32 ParameterIsUserSymbol(INT8 *str, UINT32 *SymIndex, UINT32 *SymValue)
{
	UINT32	i;

	for(i = 0; i < MAX_USER_SYMBOL_SIZE; i++)
	{
		if(UserSymbolTable[i].IsFree == FALSE)
		{
			if(!strcmp(str, UserSymbolTable[i].SymName))
			{
				if(SymIndex != NULL)
					*SymIndex = i;
				if(SymValue != NULL)
					*SymValue = UserSymbolTable[i].SymValue;
				return TRUE;
			}
		}
	}

	return FALSE;
}

/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
UINT32 ParameterIsLogicalOperation(INT8 *str, UINT32 *ul)
{
	if(str == NULL)
		return FALSE;

	if(!strcmp(str, ">"))
	{
		if(ul != NULL)
			*ul = IS_HIGHER;
		return TRUE;
	}
	else if(!strcmp(str, ">="))
	{
		if(ul != NULL)
			*ul = IS_HIGHER_SAME;
		return TRUE;
	}
	else if(!strcmp(str, "=="))
	{
		if(ul != NULL)
			*ul = IS_EQUAL;
		return TRUE;
	}
	else if(!strcmp(str, "<="))
	{
		if(ul != NULL)
			*ul = IS_LOWER_SAME;
		return TRUE;
	}
	else if(!strcmp(str, "<"))
	{
		if(ul != NULL)
			*ul = IS_LOWER;
		return TRUE;
	}
	else if(!strcmp(str, "!="))
	{
		if(ul != NULL)
			*ul = IS_NOT_EQUAL;
		return TRUE;
	}
	return FALSE;
}

/************************************************************************************************/
/************************************************************************************************/
UINT32 ParameterIsArithmeticOperation(INT8 *str, UINT32 *ul)
{
	if(str == NULL)
		return FALSE;

	if(!strcmp(str, "="))
	{
		if(ul != NULL)
			*ul = EQUAL_CMD;
		return TRUE;
	}
	else if(!strcmp(str, "+="))
	{
		if(ul != NULL)
			*ul = ADD_CMD;
		return TRUE;
	}
	else if(!strcmp(str, "-="))
	{
		if(ul != NULL)
			*ul = SUB_CMD;
		return TRUE;
	}
	else if(!strcmp(str, "*="))
	{
		if(ul != NULL)
			*ul = MUL_CMD;
		return TRUE;
	}
	else if(!strcmp(str, "/="))
	{
		if(ul != NULL)
			*ul = DIV_CMD;
		return TRUE;
	}
	else if(!strcmp(str, "%="))
	{
		if(ul != NULL)
			*ul = MOD_CMD;
		return TRUE;
	}
	return FALSE;
}

/************************************************************************************************/
/************************************************************************************************/
BOOL IsDigit(INT8 ch)
{
	if((ch >= '0') && (ch <= '9'))
		return TRUE;
	return FALSE;
}

/************************************************************************************************/
/************************************************************************************************/
BOOL IsXDigit(INT8 ch)
{
	if((ch >= '0') && (ch <= '9'))
		return TRUE;
	if((ch >= 'A') && (ch <= 'F'))
		return TRUE;
	if((ch >= 'a') && (ch <= 'f'))
		return TRUE;
	return FALSE;
}

/************************************************************************************************/
/************************************************************************************************/
UINT32 ParameterIsConstant(INT8 *str, UINT32 *ul)
{	
	UINT32	i;

	if(str == NULL)
		return FALSE;

	if(!strncmp(str, "0x", 2))
	{
		for(i = 2; str[i] != '\0'; i++)
		{
			if(!IsXDigit(str[i]))
				return FALSE;
		}
		if(ul != NULL)
			*ul = strtoul(str+2, NULL, 16);
	}
	else
	{
		for(i = 0; str[i] != '\0'; i++)
		{
			if(!IsDigit(str[i]))
				return FALSE;
		}
		if(ul != NULL)
			*ul = strtoul(str, NULL, 10);
	}

	return TRUE;
}

/************************************************************************************************/
/* Memory address should be like mw(addr) or mh(addr) or mb(addr)								*/
/************************************************************************************************/
UINT32 ParameterIsMemory(INT8 *str, UINT32 *MemValue)
{
	INT8	ptr[MAX_COMMAND_LEN], *addr_ptr;
	UINT32	addr;

	strcpy(ptr, str);

	/* if string is end with ')' */
	if(ptr[strlen(ptr) - 1] != ')')
		return FALSE;

	/* get addr form str */
	ptr[strlen(ptr) - 1] = '\0';
	addr_ptr = ptr + 3;

	/* check if addr is user-defined symbol or constant */
	if(!ParameterIsUserSymbol(addr_ptr, NULL, &addr))
		if(!ParameterIsConstant(addr_ptr, &addr))
			return FALSE;

	/* if parameter is starting with "mw(" */
	if(!strncmp(ptr, "mw(", 3))
	{
		*MemValue = (UINT32)(*((UINT32 *)addr));
		return TRUE;
	}
	else if(!strncmp(ptr, "mh(", 3))
	{
		*MemValue = (UINT32)(*((UINT16 *)addr));
		return TRUE;
	}
	else if(!strncmp(ptr, "mb(", 3))
	{
		*MemValue = (UINT32)(*((UINT8 *)addr));
		return TRUE;
	}
	return FALSE;
}	

/************************************************************************************************/
/************************************************************************************************/
UINT32 ParameterToNumber(INT8 *para, UINT32 para_type, UINT32 *value)
{
	if(para_type & IS_NUM)
	{
		/* if parameter is number */
		if(ParameterIsConstant(para, value))
			return TRUE;
	}

	if(para_type & IS_MEM)
	{
		/* if parameter is memory address */
		if(ParameterIsMemory(para, value))
			return TRUE;
	}

	if(para_type & IS_USER_SYMBOL)
	{
		if(ParameterIsUserSymbol(para, NULL, value))
			return TRUE;
	}

	return FALSE;
}

/************************************************************************************************/
/************************************************************************************************/
UINT32 ParseCommandParameter(INT8 *cmd, UINT32 index, CommandTableStruct_T CmdTable[])
{
	INT8 *CmdName, *token, *token1;
	UINT32 OptionIndex, SubOptionIndex;

	CmdName = strtok(cmd, DefaultStrDelimit);

	while((token = strtok(NULL, DefaultStrDelimit)) != NULL)
	{
		if((OptionIndex = FindOption(index, token, CmdTable)) != -1)
		{
			if(CmdTable[index].Opt[OptionIndex].Type != IS_NULL)
			{
				if((token1 = strtok(NULL, DefaultStrDelimit)) == NULL)
					return FALSE;
				CmdTable[index].Opt[OptionIndex].Str = token1;
				if((CmdTable[index].Opt[OptionIndex].Type & (IS_NUM | IS_REG | IS_MEM)) != 0)
				{
					if(!ParameterToNumber(token1, CmdTable[index].Opt[OptionIndex].Type, &CmdTable[index].Opt[OptionIndex].Value))
						return FALSE;
				}
				else if((CmdTable[index].Opt[OptionIndex].Type & IS_STR_WITH_OPTION) == IS_STR_WITH_OPTION)
				{
					if((SubOptionIndex = FineSubOption(token1, CmdTable[index].Opt[OptionIndex].SubName)) == -1)
						return FALSE;
					CmdTable[index].Opt[OptionIndex].Value = SubOptionIndex;
	 			}
			}
			else
			{
				CmdTable[index].Opt[OptionIndex].Value = TRUE;
			}

			CmdTable[index].Opt[OptionIndex].IsSet = TRUE;
			CmdTable[index].OptCount++;
		}
		else
		{
			if(!SaveArgument(index, token, CmdTable))
				return FALSE;
		}
	}

	if(CmdTable[index].ArguCount < CmdTable[index].ArguMinCount)
		return FALSE;

	if(CmdTable[index].OptCount < CmdTable[index].OptMinCount)
		return FALSE;

	return TRUE;
}

/************************************************************************************************/
/************************************************************************************************/
UINT32 SaveArgument(UINT32 index, INT8 *ArgumentStr, CommandTableStruct_T CmdTable[])
{
	UINT32 i;

	if(CmdTable[index].ArguCount >= CmdTable[index].ArguMaxCount)
		return FALSE;

	i = CmdTable[index].ArguCount;
	CmdTable[index].Argu[i].Str = ArgumentStr;
	if(CmdTable[index].Argu[i].Type == IS_ARITHMETIC_OP_STR)
	{
		if(!ParameterIsArithmeticOperation(CmdTable[index].Argu[i].Str, &CmdTable[index].Argu[i].Value))
			return FALSE;
	}
	else if(CmdTable[index].Argu[i].Type == IS_LOGICAL_OP_STR)
	{
		if(!ParameterIsLogicalOperation(CmdTable[index].Argu[i].Str, &CmdTable[index].Argu[i].Value))
			return FALSE;
	}
	else if((CmdTable[index].Argu[i].Type & (IS_NUM | IS_REG | IS_MEM)) != 0)
	{
		if(!ParameterToNumber(CmdTable[index].Argu[i].Str, CmdTable[index].Argu[i].Type, &CmdTable[index].Argu[i].Value))
			return FALSE;
	}
	CmdTable[index].Argu[i].IsSet = TRUE;
	CmdTable[index].ArguCount++;
	return TRUE;
}

/************************************************************************************************/
/************************************************************************************************/
UINT32 FindOption(UINT32 index, INT8 *OptionName, CommandTableStruct_T CmdTable[])
{
	UINT32 i;

	for(i = 0; i < CmdTable[index].OptMaxCount; i++)
	{
		if(!strcmp(CmdTable[index].Opt[i].Name, OptionName))
		{
			return i;
		}
	}
	return -1;
}

/************************************************************************************************/
/************************************************************************************************/
UINT32 FineSubOption(INT8 *Str, INT8 *SubOption[])
{
	UINT32 i;

	for(i = 0; i < CMD_STR_SUB_OPTION_COUNT; i++)
	{
		if(SubOption[i] == NULL)
			break;

//		if(!stricmp(Str, SubOption[i]))
		if(!strcmp(Str, SubOption[i]))
			return i;
	}
	return -1;
}

/************************************************************************************************/
/************************************************************************************************/
void FreeCmdArgumentOption(UINT32 index, CommandTableStruct_T CmdTable[])
{
	UINT32 i;

	CmdTable[index].ArguCount = 0;
	for(i = 0; i < CmdTable[index].ArguMaxCount; i++)
	{
		CmdTable[index].Argu[i].Str = NULL;
		CmdTable[index].Argu[i].Value = 0;
		CmdTable[index].Argu[i].IsSet = FALSE;
	}

	CmdTable[index].OptCount = 0;
	for(i = 0; i < CmdTable[index].OptMaxCount; i++)
	{
		CmdTable[index].Opt[i].Str = NULL;
		CmdTable[index].Opt[i].Value = 0;
		CmdTable[index].Opt[i].IsSet = FALSE;
	}
}


/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
UINT32 HelpCmd(CommandTableStruct_T *CmdTable)
{
	UINT32 Index, i;
	CommandTableStruct_T *pCmd;	 
		 
	/* the [command] may be omitted, so we must make sure this argument is specified */
	if(CmdTable->Argu[0].IsSet)
	{
		if((Index = ParseCommand(CmdTable->Argu[0].Str, fLib_CmdTable)) != -1)
		{
			pCmd = &fLib_CmdTable[Index];
			printf(pCmd->HelpStr);
			
			for(i = 0; i < pCmd->ArguMaxCount; i++)
				printf(pCmd->Argu[i].HelpStr);
				
			for(i = 0; i < pCmd->OptMaxCount; i++)
				printf(pCmd->Opt[i].HelpStr);

			return OK_COMMAND;
		}
	}
	else
	{
		i = 0;
		while(fLib_CmdTable[i].Name[0] != NULL)
		{
			printf(fLib_CmdTable[i].HelpStr);
			i++;
		}
		return OK_COMMAND;
	}
	return NEED_HELP_COMMAND;
}

UINT32 IfCmd(CommandTableStruct_T *CmdTable)
{
	UINT32 	Operand1, Operand2;

	/* get the operand 1, operand 2 */
	Operand1 = CmdTable->Argu[0].Value;
	Operand2 = CmdTable->Argu[2].Value;

	switch(CmdTable->Argu[1].Value)
	{
		case IS_HIGHER:
			if(Operand1 > Operand2)
				return OK_COMMAND;
			return IF_ERROR_COMMAND;
		case IS_HIGHER_SAME:
			if(Operand1 >= Operand2)
				return OK_COMMAND;
			return IF_ERROR_COMMAND;
		case IS_EQUAL:
			if(Operand1 == Operand2)
				return OK_COMMAND;
			return IF_ERROR_COMMAND;
		case IS_LOWER_SAME:
			if(Operand1 <= Operand2)
				return OK_COMMAND;
			return IF_ERROR_COMMAND;
		case IS_LOWER:
			if(Operand1 < Operand2)
				return OK_COMMAND;
			return IF_ERROR_COMMAND;
		case IS_NOT_EQUAL:
			if(Operand1 != Operand2)
				return OK_COMMAND;
			return IF_ERROR_COMMAND;
	}
	return NEED_HELP_COMMAND;
}

UINT32 WhileCmd(CommandTableStruct_T *CmdTable)
{
	UINT32 	Operand1, Operand2;

	/* get the operand 1, operand 2 */
	Operand1 = CmdTable->Argu[0].Value;
	Operand2 = CmdTable->Argu[2].Value;

	switch(CmdTable->Argu[1].Value)
	{
		case IS_HIGHER:
			if(Operand1 > Operand2)
				return WHILE_COMMAND;
			return OK_COMMAND;
		case IS_HIGHER_SAME:
			if(Operand1 >= Operand2)
				return WHILE_COMMAND;
			return OK_COMMAND;
		case IS_EQUAL:
			if(Operand1 == Operand2)
				return WHILE_COMMAND;
			return OK_COMMAND;
		case IS_LOWER_SAME:
			if(Operand1 <= Operand2)
				return WHILE_COMMAND;
			return OK_COMMAND;
		case IS_LOWER:
			if(Operand1 < Operand2)
				return WHILE_COMMAND;
			return OK_COMMAND;
		case IS_NOT_EQUAL:
			if(Operand1 != Operand2)
				return WHILE_COMMAND;
			return OK_COMMAND;
	}
	return NEED_HELP_COMMAND;
}


UINT32 SymbolCmd(CommandTableStruct_T *CmdTable)
{
	INT8				*SymName;
	UINT32 				SymIndex, SymValue, SymNum;
	CmdArguStruct_T		*SymArgu;
	CmdOptStruct_T		*DelOpt, *HexOpt;

	SymArgu = &CmdTable->Argu[0];
	DelOpt = &CmdTable->Opt[0];
	HexOpt = &CmdTable->Opt[1];
	 
	/* if [symbol] is omitted but [-d] is specified */
	if(!SymArgu->IsSet && DelOpt->IsSet)
		return NEED_HELP_COMMAND;

	/* the [symbol] is specified */
	if(SymArgu->IsSet)
	{
		/* if [-d] is specified, delete it */
		if(DelOpt->IsSet)
		{
			if(!ParameterIsUserSymbol(SymArgu->Str, &SymIndex, NULL))
			{
				printf("The symbol does not exist!!! Can not be deleted!!!\n");
				return ERROR_COMMAND;
			}

			if(DeleteUserSymbol(SymIndex))
				printf("The symbol has been deleted!!!\n");					
			else
				printf("The symbol can not be deleted!!!\n");					
		}
		else
		{
			if(ParameterIsUserSymbol(SymArgu->Str, &SymIndex, NULL))
			{
				printf("The symbol already exists!!! Can not be created!!!\n");					
				return ERROR_COMMAND;
			}

			if(!NewUserSymbol(SymArgu->Str))
			{
				printf("Too many symbols!!! Can not be created!!!\n");					
				return ERROR_COMMAND;
			}

			printf("The symbol has been created!!!\n");					
		}
	}
	else
	{
		/* the [symbol] is not specified and the [-d] is not specified, display all user-defined symbol */
		SymNum = 0;
		for(SymIndex = 0; SymIndex < MAX_USER_SYMBOL_SIZE; SymIndex++)
		{
			if((SymName = ReadUserSymbol(SymIndex, &SymValue)) != NULL)
			{
				printf("%s = 0x%x\n", SymName, SymValue);					
				SymNum++;
			}
		}
		printf("Total %d user-defined symbols.\n", SymNum);					
	}
	return OK_COMMAND;
}

/************************************************************************************************/
/************************************************************************************************/
UINT32 UserSymbolCmd(CommandTableStruct_T *CmdTable)
{
	char	*SymName;
	UINT32 	SymValue;

	SymName = ReadUserSymbol(CmdTable->Key, &SymValue);

	/* if the [value] is specified */
	if(CmdTable->Argu[1].IsSet)
	{		
		switch(CmdTable->Argu[0].Value)
		{
			case EQUAL_CMD:	
				SymValue = CmdTable->Argu[1].Value;
				break;
			case ADD_CMD:
				SymValue += CmdTable->Argu[1].Value;
				break;
			case SUB_CMD:
				SymValue -= CmdTable->Argu[1].Value;
				break;
			case MUL_CMD:
				SymValue *= CmdTable->Argu[1].Value;
				break;
			case DIV_CMD:
				SymValue /= CmdTable->Argu[1].Value;
				break;
			case MOD_CMD:
				SymValue %= CmdTable->Argu[1].Value;
				break;
		}
		WriteUserSymbol(CmdTable->Key, SymValue);
	}
	if(CmdTable->Opt[0].IsSet)
		printf("%s = 0x%x\n", SymName, SymValue);		
	else
		printf("%s = %d\n", SymName, SymValue);		

	return OK_COMMAND;
}

