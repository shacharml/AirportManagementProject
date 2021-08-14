#ifndef __COMP__
#define __COMP__

#include "Flight.h"
#include "AirportManager.h"
#include "GeneralList.h"
#include "myMacros.h"

typedef unsigned char BYTE;

typedef enum { eNone, eHour, eDate, eSorceCode, eDestCode, eNofSortOpt } eSortOption;
static const char* sortOptStr[eNofSortOpt];

#define DETAIL_PRINT
#define BIT_COMP

typedef struct
{
	char*		name;
	int			flightCount;
	Flight**	flightArr;
	eSortOption	sortOpt;
	LIST		flighDateList;
}Company;

int		initCompanyFromFile(Company* pComp, AirportManager* pManaer, const char* fileName);
void	initCompany(Company* pComp, AirportManager* pManaer);
void	initDateList(Company* pComp);
int		isUniqueDate(const Company* pComp, int index);
int		addFlight(Company* pComp, const AirportManager* pManager);

void	printCompany(const Company* pComp, const char* newSrt);
void	printFlightsCount(const Company* pComp);

int		saveCompanyToFile(const Company* pComp, const char* fileName);
int		loadCompanyFromFile(Company* pComp, const AirportManager* pManager, const char* fileName);

void	sortFlight(Company* pComp);
void	findFlight(const Company* pComp);
eSortOption showSortMenu();

int	saveCompanyInBite(const Company* pComp, const char* fileName);
int	loadCompanyInBite(Company* pComp, const AirportManager* pManager, const char* fileName);

void	freeCompany(Company* pComp);

#endif

