#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "Company.h"
#include "Airport.h"
#include "General.h"
#include "fileHelper.h"

static const char* sortOptStr[eNofSortOpt] = {
	"None","Hour", "Date", "Airport takeoff code", "Airport landing code" };


int	initCompanyFromFile(Company* pComp, AirportManager* pManaer, const char* fileName)
{
	L_init(&pComp->flighDateList);
	if (loadCompanyFromFile(pComp, pManaer, fileName))
	{
		initDateList(pComp);
		return 1;
	}
	return 0;
}

void	initCompany(Company* pComp, AirportManager* pManaer)
{
	printf("-----------  Init Airline Company\n");
	L_init(&pComp->flighDateList);

	pComp->name = getStrExactName("Enter company name");
	pComp->flightArr = NULL;
	pComp->flightCount = 0;
}

void	initDateList(Company* pComp)
{
	for (int i = 0; i < pComp->flightCount; i++)
	{
		if (isUniqueDate(pComp, i))
		{
			char* sDate = createDateString(&pComp->flightArr[i]->date);
			L_insert(&(pComp->flighDateList.head), sDate);
		}
	}
}

int		isUniqueDate(const Company* pComp, int index)
{
	Date* pCheck = &pComp->flightArr[index]->date;
	for (int i = 0; i < index; i++)
	{
		if (equalDate(&pComp->flightArr[i]->date, pCheck))
			return 0;
	}
	return 1;
}

int		addFlight(Company* pComp, const AirportManager* pManager)
{

	if (pManager->count < 2)
	{
		printf("There are not enoght airport to set a flight\n");
		return 0;
	}
	pComp->flightArr = (Flight**)realloc(pComp->flightArr, (pComp->flightCount + 1) * sizeof(Flight*));
	CHECK_RETURN_0(pComp->flightArr);

	pComp->flightArr[pComp->flightCount] = (Flight*)calloc(1, sizeof(Flight));
	if (!pComp->flightArr[pComp->flightCount])
		return 0;
	initFlight(pComp->flightArr[pComp->flightCount], pManager);
	if (isUniqueDate(pComp, pComp->flightCount))
	{
		char* sDate = createDateString(&pComp->flightArr[pComp->flightCount]->date);
		L_insert(&(pComp->flighDateList.head), sDate);
	}
	pComp->flightCount++;
	return 1;
}

void	printCompany(const Company* pComp, const char* strNew)
{
	char* newName = (char*)malloc((strlen(strNew) + strlen(pComp->name) + 2) * sizeof(char));

	strcpy(newName, pComp->name);
	strcat(newName, "_");
	strcat(newName, strNew);

	printf("Company %s:\n", newName);
	printf("Has %d flights\n", pComp->flightCount);

#ifdef DETAIL_PRINT
	generalArrayFunction((void*)pComp->flightArr, pComp->flightCount, sizeof(Flight**), printFlightV);
	printf("\nFlight Date List:");
	L_print(&pComp->flighDateList, printStr);
#endif

	free(newName);
}

void	printFlightsCount(const Company* pComp)
{
	char codeOrigin[CODE_LENGTH + 1];
	char codeDestination[CODE_LENGTH + 1];

	if (pComp->flightCount == 0)
	{
		printf("No flight to search\n");
		return;
	}

	printf("Origin Airport\n");
	getAirportCode(codeOrigin);
	printf("Destination Airport\n");
	getAirportCode(codeDestination);

	int count = countFlightsInRoute(pComp->flightArr, pComp->flightCount, codeOrigin, codeDestination);
	if (count != 0)
		printf("There are %d flights ", count);
	else
		printf("There are No flights ");

	printf("from %s to %s\n", codeOrigin, codeDestination);
}

int		saveCompanyToFile(const Company* pComp, const char* fileName)
{

#ifdef BIT_COMP

	if (!(saveCompanyInBite(pComp, fileName)))
		return 0;

#else
	FILE* fp;
	fp = fopen(fileName, "wb");
	CHECK_MSG_RETURN_0(fp, Error open copmpany file to write); ///3


	if (!writeStringToFile(pComp->name, fp, "Error write comapny name\n"))
		return 0;


	if (!writeIntToFile(pComp->flightCount, fp, "Error write flight count\n"))
		return 0;

	if (!writeIntToFile((int)pComp->sortOpt, fp, "Error write sort option\n"))
		return 0;

	for (int i = 0; i < pComp->flightCount; i++)
	{
		if (!saveFlightToFile(pComp->flightArr[i], fp))
			return 0;
	}

	fclose(fp);
#endif //BIT_COMP
	return 1;
}

int loadCompanyFromFile(Company* pComp, const AirportManager* pManager, const char* fileName)
{
#ifdef BIT_COMP
	if (!(loadCompanyInBite(pComp, pManager, fileName))) 
		return 0;

#else
	FILE* fp;
	fp = fopen(fileName, "rb");
	if (!fp)
	{
		printf("Error open company file\n");
		return 0;
	}

	pComp->flightArr = NULL;


	pComp->name = readStringFromFile(fp, "Error reading company name\n");
	if (!pComp->name)
		return 0;

	if (!readIntFromFile(&pComp->flightCount, fp, "Error reading flight count name\n"))
		return 0;

	int opt;
	if (!readIntFromFile(&opt, fp, "Error reading sort option\n"))
		return 0;

	pComp->sortOpt = (eSortOption)opt;

	if (pComp->flightCount > 0)
	{
		pComp->flightArr = (Flight**)malloc(pComp->flightCount * sizeof(Flight*));

		CHECK_NULL_MSG_COLSE_FILE(pComp->flightArr, fp, Alocation error);
	}
	else
		pComp->flightArr = NULL;

	for (int i = 0; i < pComp->flightCount; i++)
	{
		pComp->flightArr[i] = (Flight*)calloc(1, sizeof(Flight));

		CHECK_NULL_MSG_COLSE_FILE(pComp->flightArr[i], fp, Alocation error);
		if (!loadFlightFromFile(pComp->flightArr[i], pManager, fp))
			return 0;
	}

	fclose(fp);
#endif
	return 1;
}

void	sortFlight(Company* pComp)
{
	pComp->sortOpt = showSortMenu();
	int(*compare)(const void* air1, const void* air2) = NULL;

	switch (pComp->sortOpt)
	{
	case eHour:
		compare = compareByHour;
		break;
	case eDate:
		compare = compareByDate;
		break;
	case eSorceCode:
		compare = compareByCodeOrig;
		break;
	case eDestCode:
		compare = compareByCodeDest;
		break;

	}

	if (compare != NULL)
		qsort(pComp->flightArr, pComp->flightCount, sizeof(Flight*), compare);

}

void	findFlight(const Company* pComp)
{
	int(*compare)(const void* air1, const void* air2) = NULL;
	Flight f = { 0 };
	Flight* pFlight = &f;


	switch (pComp->sortOpt)
	{
	case eHour:
		f.hour = getFlightHour();
		compare = compareByHour;
		break;
	case eDate:
		getchar();
		getCorrectDate(&f.date);
		compare = compareByDate;
		break;
	case eSorceCode:
		getchar();
		getAirportCode(f.originCode);
		compare = compareByCodeOrig;
		break;
	case eDestCode:
		getchar();
		getAirportCode(f.destCode);
		compare = compareByCodeDest;
		break;
	}

	if (compare != NULL)
	{
		Flight** pF = bsearch(&pFlight, pComp->flightArr, pComp->flightCount, sizeof(Flight*), compare);
		if (pF == NULL)
			printf("Flight was not found\n");
		else {
			printf("Flight found, ");
			printFlight(*pF);
		}
	}
	else {
		printf("The search cannot be performed, array not sorted\n");
	}

}

eSortOption showSortMenu()
{
	int opt;
	printf("Base on what field do you want to sort?\n");
	do {
		for (int i = 1; i < eNofSortOpt; i++)
			printf("Enter %d for %s\n", i, sortOptStr[i]);
		scanf("%d", &opt);
	} while (opt < 0 || opt >eNofSortOpt);

	return (eSortOption)opt;
}

int saveCompanyInBite(const Company * pComp, const char * fileName)
{
	FILE* fp = fopen(fileName, "wb");
	if (!fp)
	{
		printf("Error open file \n");
		return 0;
	}

	Flight* pFlight;
	BYTE companyData[2] = { 0 };
	BYTE flightData[4] = { 0 };

	companyData[0] |= (strlen(pComp->name) & createMask(3, 0));
	companyData[0] |= ((pComp->sortOpt & createMask(2, 0)) << 4);
	companyData[0] |= ((pComp->flightCount & 1) << 7); 
	companyData[1] |= (pComp->flightCount >> 1);

	if (fwrite(companyData, sizeof(BYTE), 2, fp) != 2)
	{
		fclose(fp);
		return 0;
	}
	if (fwrite(pComp->name, sizeof(char), strlen(pComp->name) + 1, fp) != strlen(pComp->name) + 1)
	{
		fclose(fp);
		return 0;
	}
	
	for (int i = 0; i < pComp->flightCount; i++)
	{
		pFlight = pComp->flightArr[i];

		if (fwrite(pFlight->originCode, sizeof(char), CODE_LENGTH, fp) != CODE_LENGTH)
		{
			fclose(fp);
			return 0;
		}
		if (fwrite(pFlight->destCode, sizeof(char), CODE_LENGTH, fp) != CODE_LENGTH)
		{
			fclose(fp);
			return 0;
		}

		for (int j = 0; j < 4; j++) // needs always that the flightData be inilaiz in zeros
			flightData[j] &= 0;

		flightData[3] |= (pFlight->date.year >> 10);
		flightData[2] |= ((pFlight->date.year >> 2) & createMask(7, 0));
		flightData[1] |= ((pFlight->date.year & createMask(1, 0)) << 6);
		flightData[1] |= (pFlight->date.month << 2);
		flightData[1] |= (pFlight->date.day >> 3);
		flightData[0] |= ((pFlight->date.day & createMask(2, 0)) << 5);
		flightData[0] |= pFlight->hour;

		if (fwrite(flightData, sizeof(BYTE), 4, fp) != 4)
		{
			fclose(fp);
			return 0;
		}
	}
	fclose(fp);
	return 1;
}

int loadCompanyInBite(Company * pComp, const AirportManager * pManager, const char * fileName)
{
	FILE* fp = fopen(fileName, "rb");
	if (!fp)
	{
		printf("Error open company file\n");
		return 0;
	}
	BYTE data[2] = { 0 };
	int lenName;
	//read the 2 bytes from the file, temp
	if (fread(data, sizeof(BYTE), 2, fp) != 2)
		return 0;
	///read the num of flight and preper storage for them in the flightArr
	pComp->flightCount = (int)((data[1] << 1) | (data[0] >> 7));
	if (pComp->flightCount > 0)
	{
		pComp->flightArr = (Flight**)malloc(pComp->flightCount * (sizeof(Flight*))); 
		if (!(pComp->flightArr))
		{
			fclose(fp);
			return 0;
		}
	}
	pComp->sortOpt = ((data[0] & createMask(6, 4)) >> 4);
	
	lenName = (int)(data[0] & createMask(3, 0));
	pComp->name = (char*)malloc((lenName + 1) * sizeof(char));
	if (!pComp->name)
	{	//aloocation didnt work
		fclose(fp);
		return 0;
	}

	///read the next bytes >> into company name
	if (fread(pComp->name, sizeof(char), lenName + 1, fp) != (lenName + 1))
	{
		fclose(fp);
		return 0;
	}

	for (int i = 0; i < pComp->flightCount; i++)
	{
		Flight* pFlight = (Flight*)malloc(sizeof(Flight));
		char origCode[CODE_LENGTH + 1] = { 0 };
		char desCode[CODE_LENGTH + 1] = { 0 };

		fread(origCode, sizeof(char), CODE_LENGTH, fp);
		fread(desCode, sizeof(char), CODE_LENGTH, fp);

		origCode[CODE_LENGTH] = '\0';
		desCode[CODE_LENGTH] = '\0';

		///check the codes in the airporst list
		if ((!(findAirportByCode(pManager, origCode))) || (!(findAirportByCode(pManager, desCode))))
		{//we dont need to continue read from the file
			fclose(fp);
			return 0;
		}
		strcpy(pFlight->originCode, origCode);
		strcpy(pFlight->destCode, desCode);

		BYTE fData[4] = { 0 };
		fread(fData, sizeof(BYTE), 4, fp);

		pFlight->date.year = (int)((fData[3] << 10) | (fData[2] << 2) | (fData[1] >> 6));
		pFlight->date.month = (int)((fData[1] & createMask(5, 2)) >> 2);
		pFlight->date.day = (int)(((fData[1] & createMask(1, 0)) << 3) | ((fData[0] & createMask(7, 5)) >> 5));
		pFlight->hour = (int)(fData[0] & createMask(4, 0));
		pComp->flightArr[i] = pFlight;
	}

	fclose(fp);
	return 1;
}

void	freeCompany(Company* pComp)
{
	generalArrayFunction((void*)pComp->flightArr, pComp->flightCount, sizeof(Flight**), freeFlight);
	free(pComp->flightArr);
	free(pComp->name);
	L_free(&pComp->flighDateList, freePtr);
}