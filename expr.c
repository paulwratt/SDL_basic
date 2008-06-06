#include <ctype.h>
#include "misc.h"

typedef unsigned char uchar;

#define EXPRMAX 128

typedef struct expr_context {
	bc *bc;
	uchar exprstack[EXPRMAX];
	uchar *exprsp;
	int exprflag;
	int oppri;
	int opop;
	double operval;
	int exprflags;
	char *pnt;
} ec;


double expr2(ec *ec);
void operand(ec *ec);
void operator(ec *ec);
int trytop(ec *ec);

void unbalancedq(ec *ec)
{
}



int at(ec *ec)
{
	return *ec->pnt;
}

int get(ec *ec)
{
	return *ec->pnt++;
}

int back(ec *ec)
{
	return *--ec->pnt;
}

int isoknum(uchar ach)
{
	return ach>='0' && ach<='9';
}


void pushl(ec *ec, double val)
{
	*(double *)ec->exprsp=val;
	ec->exprsp+=sizeof(double);
}

void pushb(ec *ec, uchar val)
{
	*ec->exprsp++ = val;
}

double popl(ec *ec)
{
	ec->exprsp-=sizeof(double);
	return *(double *)ec->exprsp;
}

int popb(ec *ec)
{
	return *--ec->exprsp;
}

double expr2(ec *ec);

double expr(bc *bc, char **take)
{
double eval;
ec ecl, *ec;
	ec = &ecl;

	ec->pnt = *take;
	ec->exprsp=ec->exprstack;
	ec->exprflag=0;
	eval = expr2(ec);
//	if(ec->exprflag & 1) unbalanced();
//	if(ec->exprflag & 2) badoperation();
	*take = ec->pnt;
	return eval;
}
/*uchar opchars[]={'+','-','/','*','|','&','<<','>>','!'};*/

double expr2(ec *ec)
{
	pushb(ec, 0);
	if(at(ec)=='-')
	{
		get(ec);
		pushl(ec, 0.0);
		pushb(ec, 1);
		pushb(ec, 0x10);
	}
	for(;;)
	{
		operand(ec);
		operator(ec);
		if(trytop(ec)) break;

		pushl(ec, ec->operval);
		pushb(ec, ec->opop);
		pushb(ec, ec->oppri);
	}
	popb(ec);
	return ec->operval;
}

int trytop(ec *ec)
{
	uchar toppri,topop;
	double topval;

	for(;;)
	{
		toppri=popb(ec);
		if(ec->oppri>toppri) {pushb(ec, toppri);return ec->oppri==8;}
		topop=popb(ec);
		topval=popl(ec);

		switch(topop)
		{
			case 0: /* + */
				ec->operval+=topval;
				break;
			case 1: /* - */
				ec->operval=topval-ec->operval;
				break;
			case 2: /* / */
				ec->operval=topval/ec->operval;
				break;
			case 3: /* * */
				ec->operval*=topval;
				break;
			case 4: /* | */
				ec->operval=(int)ec->operval | (int)topval;
				break;
			case 5: /* & */
				ec->operval=(int)ec->operval | (int)topval;
				break;
			case 6: /* << */
				ec->operval=(int)topval<<(int)ec->operval;
				break;
			case 7: /* >> */
				ec->operval=(int)topval>>(int)ec->operval;
				break;
			case 8: return 1;
		}
	}
}

void operator(ec *ec)
{
uchar ch;

	ch=get(ec);
	switch(ch)
	{
		case '+': ec->oppri=16;ec->opop=0;break;
		case '-': ec->oppri=16;ec->opop=1;break;
		case '/': ec->oppri=24;ec->opop=2;break;
		case '*': ec->oppri=24;ec->opop=3;break;
		case '|': ec->oppri=32;ec->opop=4;break;
		case '&': ec->oppri=40;ec->opop=5;break;
		case '<':
			if(get(ec)!='<') back(ec);
			ec->oppri=48;ec->opop=6;break;
		case '>':
			if(get(ec)!='>') back(ec);
			ec->oppri=32;ec->opop=7;break;
		default:
			back(ec);ec->oppri=8;ec->opop=8;
	}
}

/*
+ 010
- 110
/ 218,20f
* 318
| 420
& 528
<< 630
>> 730
. , ( ) white ; 008
*/


/* fills in operval and opertype, leaves pointer on character stopped on */
void operand(ec *ec)
{
	uchar ch;

	ch=at(ec);
	if((ch>='0' && ch<='9') || ch=='.')
	{
		double intpart;

		intpart = 0.0;
		while(isoknum(ch=get(ec))) {intpart*=10;intpart+=ch-'0';}
		if(ch=='.')
		{
			double digit=0.1;
			double fracpart=0.0;
			while(isoknum(ch=get(ec)))
			{
				fracpart += digit * (ch - '0');
				digit /= 10.0;
			}
			ec->operval = intpart + fracpart;
		} else
			ec->operval = intpart;
		back(ec);
	} else if(ch=='\'')
	{
		get(ec);
		ec->operval=0.0;
		while((ch=get(ec)))
		{
			if(ch=='\n' || !ch) {back(ec);unbalancedq(ec);break;}
			if(ch=='\'')
				if(get(ec)!='\'') {back(ec);break;}
			ec->operval*=256;ec->operval+=ch;
		}
	} else if(ch=='(')
	{
		get(ec);
		ec->operval=expr2(ec);
		if(get(ec)!=')') {ec->exprflag|=1;back(ec);}
	}  /*else
	{
		ec->operval=0.0;
		while(ishex(ch=get(ec))) {ec->operval*=16;ec->operval+=tohex(ch);}
		back(ec);
	}*/
}