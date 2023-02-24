/*
	---------------------------------------------------------------
	Algorithm used: FCFS and Priority
	---------------------------------------------------------------
	code compiled under GCC4.8
*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// maximum events
const int MAX_EVENT = 400;

// time slot of all devices and rooms for a day
struct time_slot{
	int roomA[24];
	int roomB[24];
	int roomC[24];
	int webcam_FHD_1[24];
	int webcam_FHD_2[24];
	int webcam_UHD[24];
	int monitor_50_1[24];
	int monitor_50_2[24];
	int monitor_75[24];
	int projector_2k_1[24];
	int projector_2k_2[24];
	int projector_4k[24];
	int screen_100_1[24];
	int screen_100_2[24];
	int screen_150[24];	
};

// one line of command = one event
// event include add* and booking
struct Event{   	
    char event[15];
    char tenant[10];
    struct tm mtm;
	int dateSlot;
	int timeSlot;
    float duration;
    int persons;
    char deviceA[20];
    char deviceB[20];
	bool accepted_state;
	char room;
	bool validaty;
    int event_num;
};

//calculate end time with start time & duration
struct tm handleDateTime(struct tm mtm, float duration){
	mtm.tm_min += (int)(duration * 60);
	while(mtm.tm_min > 59){
	    mtm.tm_min -= 60;
	    mtm.tm_hour += 1;
	}
	while(mtm.tm_hour > 23){
		mtm.tm_hour -= 24;
	    mtm.tm_mday += 1;
	}
	
	return mtm;
}

struct Event createEvent(char **inputCommand_ptr, int i) {
	char *event_get;
	bool invalid;
	if (strncmp(inputCommand_ptr[0],"add",3) == 0) {       
		// print size     

		// add NULL to last position of words
		// inputCommand_ptr[2][4] = inputCommand_ptr[2][7] = '\0';
		// inputCommand_ptr[3][2] = '\0';

		// store the add* command and check if it is valid
		if (strcmp(inputCommand_ptr[0],"addMeeting") == 0) {
			printf("Command is addMeeting.\n");                
			event_get = "Meeting";
			// promt user if there is too many or too few arguments (at least 6 and at most 8)
			if (!(i == 6 || i == 8)) {
				printf("Invalid command (wrong number of input arguments).\n");
				invalid = true;
			} 
		} 
		else if (strcmp(inputCommand_ptr[0],"addPresentation") == 0) {
			printf("Command is addPresentation.\n");
			event_get = "Presentation";
			// mandatorily include additional devices
			if(i != 8){
				printf("Invalid command (wrong number of input arguments).\n");
				invalid = true;
			}
		}
		else if (strcmp(inputCommand_ptr[0],"addConference") == 0) {
			printf("Command is addConference.\n");
			event_get = "Conference";
			// mandatorily include additional devices
			if(i != 8){
				printf("Invalid command (wrong number of input arguments).\n");
				invalid = true;
			}
		}
		else {
			printf("Invalid command (unknown command word). Please use addPresentation, addPresentation, or addConference.\n");
		}

		if (invalid == false) {
			//initialie event data	(date, time, duration, persons)
			struct Event newEvent = {
				.mtm.tm_year = atoi(&inputCommand_ptr[2][0]),
				.mtm.tm_mon = atoi(&inputCommand_ptr[2][5]),
				.mtm.tm_mday = atoi(&inputCommand_ptr[2][8]),
				.mtm.tm_hour = atoi(&inputCommand_ptr[3][0]),
				.mtm.tm_min = atoi(&inputCommand_ptr[3][3]),
				.duration = atof(inputCommand_ptr[4]),
				.persons = atoi(inputCommand_ptr[5]),
				.accepted_state = false,
				.validaty = true,
				.room = ' '
			};		
			
			// convert day and hour to time slot position
			newEvent.dateSlot = newEvent.mtm.tm_mday;
			newEvent.timeSlot = newEvent.mtm.tm_hour;

			// strcpy of type of event, tenant, device A & B	            
			strcpy(newEvent.event,event_get);
			strcpy(newEvent.tenant,inputCommand_ptr[1]);
			if(i > 6 && inputCommand_ptr[6] != NULL) 
				strcpy(newEvent.deviceA,inputCommand_ptr[6]);
			else
				strcpy(newEvent.deviceA,"*");
			
			if (strcmp(newEvent.deviceA,"*") == 0)
				strcpy(newEvent.deviceB,"*");
			else
				strcpy(newEvent.deviceB,inputCommand_ptr[7]);
							
			printf("*****device A is %s\n",newEvent.deviceA);
			printf("*****device B is %s\n",newEvent.deviceB);
		
			// return the event back to main
			return newEvent;
		}
		
	}

	// book device
	else if (strcmp(inputCommand_ptr[0],"bookDevice") == 0) {
		printf("Command is bookDevice.\n");
		
		if(i != 6){
				printf("Invalid command (wrong number of input arguments).\n");
				invalid = true;
		}
		
		if(invalid == false){
			//initialie event data	(date, time, duration, persons)
			struct Event newEvent = {
				.mtm.tm_year = atoi(&inputCommand_ptr[2][0]),
				.mtm.tm_mon = atoi(&inputCommand_ptr[2][5]),
				.mtm.tm_mday = atoi(&inputCommand_ptr[2][8]),
				.mtm.tm_hour = atoi(&inputCommand_ptr[3][0]),
				.mtm.tm_min = atoi(&inputCommand_ptr[3][3]),
				.duration = atof(inputCommand_ptr[4]),
				.validaty = true
			};
		
			// convert day and hour to time slot position
			newEvent.dateSlot = newEvent.mtm.tm_mday;
			newEvent.timeSlot = newEvent.mtm.tm_hour;
		
			// strcpy of type of event, tenant, and device            
			strcpy(newEvent.event,"bookDevice");
			strcpy(newEvent.tenant,inputCommand_ptr[1]);
			strcpy(newEvent.deviceA,inputCommand_ptr[5]);		
			strcpy(newEvent.deviceB,"*");
		
			// return the event back to main
			return newEvent;
		}
	
	}
	
	struct Event newEvent = {
		.validaty = false
	};
	// return the event back to main
	return newEvent;
}


void printEvent(struct Event e, int event_num) {
	//print screen for test
	printf("Test print event!\n");
	printf("Event number: %i\n",e.event_num);
	printf("Event: %s\n",e.event);
	printf("Tenant: %s\n",e.tenant);
	printf("Time: %04d-%02d-%02d %02d:%02d\n",e.mtm.tm_year,
	e.mtm.tm_mon,e.mtm.tm_mday,e.mtm.tm_hour,e.mtm.tm_min);
	printf("Duration: %.1f hour(s)\n",e.duration);
	struct tm endTime = handleDateTime(e.mtm, e.duration);
	printf("End Time: %04d-%02d-%02d %02d:%02d\n",endTime.tm_year,
	endTime.tm_mon,endTime.tm_mday,endTime.tm_hour,endTime.tm_min);
	printf("Num of persons: %d\n",e.persons);
	printf("Device A: %s\n",e.deviceA);
	printf("Device B: %s\n",e.deviceB);
	printf("Date slot: %i\n",e.dateSlot);
	printf("Time slot: %i\n",e.timeSlot);
	printf("\n");
}

void printTimeSlot(struct time_slot test, int c) {
	printf("This is time slot for child %i\n-----------------------------------------------\n", c);

	int i;
	for (i = 0; i < 24; i++) {
		printf("%2i:00 | %2i %2i %2i %2i %2i %2i %2i %2i %2i %2i %2i %2i %2i %2i %2i\n", i,
		test.roomA[i],test.roomB[i],test.roomC[i],test.webcam_FHD_1[i],test.webcam_FHD_2[i],
		test.webcam_UHD[i],test.monitor_50_1[i],test.monitor_50_2[i],test.monitor_75[i],test.projector_2k_1[i],
		test.projector_2k_2[i],test.projector_4k[i],test.screen_100_1[i],test.screen_100_2[i],test.screen_150[i]
		);
	}
}

void printByTenants(struct Event* eventList, char *algorithm, int total_events) {
	
	int day, hour;
	int j, t;
	char tenants[5][10] = {"-tenant_A","-tenant_B","-tenant_C","-tenant_D","-tenant_E"};
	int tenant_events[5] = {0,0,0,0,0};
	int accepted_events[5] = {0,0,0,0,0};
	int rejected_events[5] = {0,0,0,0,0};

	FILE *fp = fopen("RBM_Report_G16.txt", "a");
            		
    //test input
	// addMeeting -tenant_A 2021-05-16 10:00 3.0 9 projector_2K screen_100;
	// bookDevice -tenant_C 2021-05-11 13:00 4.0 projector_4K;
	// addConference -tenant_E 2021-05-16 14:00 2.0 15 webcam_UHD monitor_75;
	// addPresentation -tenant_B 2021-05-14 08:00 3.0 12 projector_4K screen_150;          		
	// addMeeting -tenant_A 2021-05-16 10:00 3.0 9 projector_2K screen_100;
	// addMeeting -tenant_C 2021-05-16 10:00 3.0 9 projector_2K screen_100;
	// addBatch -batch001.dat
	// printBookings -fcfs

	// for accepted

	// print the title
	if (strncmp(algorithm,"fcfs",4) == 0) {
		printf("*** Room Booking - ACCEPTED / FCFS ****\n\n");
		fprintf(fp,"*** Room Booking - ACCEPTED / FCFS ****\n\n");
	}

	if (strncmp(algorithm,"prio",4) == 0) {
		printf("*** Room Booking - ACCEPTED / Priority ****\n\n");
		fprintf(fp,"*** Room Booking - ACCEPTED / Priority ****\n\n");
	}

	if (strncmp(algorithm,"opti",4) == 0) {
		printf("*** Room Booking - ACCEPTED / Optimal ****\n\n");
		fprintf(fp,"*** Room Booking - ACCEPTED / Optimal ****\n\n");
	}

	if (strncmp(algorithm,"ALL",3) == 0) {
		printf("*** Room Booking - ACCEPTED / ALL ****\n\n");
		fprintf(fp,"*** Room Booking - ACCEPTED / ALL ****\n\n");
	}

	// print the result seperated by tenants
	for (t = 0; t < 5; t++) {            			
		char *tenantText = (char *)malloc(10);
		strcpy(tenantText,tenants[t]);
		tenantText++;
		tenantText[0] = toupper(tenantText[0]);

		// write to console
		printf("%s has the following bookings:\n\n",tenantText);
		printf("Date         Start   End     Type          Room       Device\n");
		printf("===========================================================================\n");

		// write to file
		fprintf(fp,"%s has the following bookings:\n\n",tenantText);
		fprintf(fp,"Date         Start   End     Type          Room       Device\n");
		fprintf(fp,"===========================================================================\n");
		
		// printing accetped, ordered by day then start hour
		for (day = 10; day < 17; day++) {
			for(hour = 0; hour < 24; hour++) {				
				for(j = 0; j < MAX_EVENT; j++) {
					if (strcmp(eventList[j].tenant,tenants[t]) == 0 && eventList[j].mtm.tm_mday == day && eventList[j].mtm.tm_hour == hour) {
						tenant_events[t]++;
						// print the event
						if (eventList[j].accepted_state == true) {
							// bookDevice do not need room
							if (strcmp(eventList[j].event,"bookDevice") == 0) {
								accepted_events[t]++;
								struct tm endTime = handleDateTime(eventList[j].mtm, eventList[j].duration);

								// write to console
								printf("%04d-%02d-%02d   %02d:%02d   %02d:%02d   %-12s  *         %s\n",
								eventList[j].mtm.tm_year,eventList[j].mtm.tm_mon,eventList[j].mtm.tm_mday,
								eventList[j].mtm.tm_hour,eventList[j].mtm.tm_min,
								endTime.tm_hour,endTime.tm_min,
								eventList[j].event,
								eventList[j].deviceA);

								// write to file
								fprintf(fp,"%04d-%02d-%02d   %02d:%02d   %02d:%02d   %-12s  *         %s\n",
								eventList[j].mtm.tm_year,eventList[j].mtm.tm_mon,eventList[j].mtm.tm_mday,
								eventList[j].mtm.tm_hour,eventList[j].mtm.tm_min,
								endTime.tm_hour,endTime.tm_min,
								eventList[j].event,
								eventList[j].deviceA);
								if(strcmp(eventList[j].deviceB, "*") != 0){
									printf("%63s\n",eventList[j].deviceB);
									fprintf(fp,"%63s\n",eventList[j].deviceB);
								}
							}
							// add* event will have a room
							else {
								accepted_events[t]++;
								struct tm endTime = handleDateTime(eventList[j].mtm, eventList[j].duration);

								// write to console
								printf("%04d-%02d-%02d   %02d:%02d   %02d:%02d   %-12s  room_%c    %s\n",
								eventList[j].mtm.tm_year,eventList[j].mtm.tm_mon,eventList[j].mtm.tm_mday,
								eventList[j].mtm.tm_hour,eventList[j].mtm.tm_min,
								endTime.tm_hour,endTime.tm_min,
								eventList[j].event,
								eventList[j].room,
								eventList[j].deviceA);

								// write to file
								fprintf(fp,"%04d-%02d-%02d   %02d:%02d   %02d:%02d   %-12s  room_%c    %s\n",
								eventList[j].mtm.tm_year,eventList[j].mtm.tm_mon,eventList[j].mtm.tm_mday,
								eventList[j].mtm.tm_hour,eventList[j].mtm.tm_min,
								endTime.tm_hour,endTime.tm_min,
								eventList[j].event,
								eventList[j].room,
								eventList[j].deviceA);

								if(strcmp(eventList[j].deviceB, "*") != 0){
									
									if(strcmp(eventList[j].deviceB, "projector_2K") == 0 || strcmp(eventList[j].deviceB, "projector_4K") == 0){
										printf("%65s\n",eventList[j].deviceB);		// write to console
										fprintf(fp,"%65s\n",eventList[j].deviceB);	// write to file
									}
									else{
										printf("%63s\n",eventList[j].deviceB);		// write to console
										fprintf(fp,"%63s\n",eventList[j].deviceB);	// write to file
									}
									
								}
							}
						}
					}
									
				}

			}
		}		
		
		tenantText--;
		printf("\n");
	}
		
	// write to console
	printf("\n- End -\n");
	printf("===========================================================================\n");

	// write to file
	fprintf(fp,"\n- End -\n");
	fprintf(fp,"===========================================================================\n");

	for (t = 0; t < 5; t++) {
		rejected_events[t] = tenant_events[t] - accepted_events[t];
	}

	// print the title
	if (strncmp(algorithm,"fcfs",4) == 0) {
		printf("*** Room Booking - REJECTED / FCFS ****\n\n");
		fprintf(fp,"*** Room Booking - REJECTED / FCFS ****\n\n");
	}

	if (strncmp(algorithm,"prio",4) == 0) {
		printf("*** Room Booking - REJECTED / Priority ****\n\n");
		fprintf(fp,"*** Room Booking - REJECTED / Priority ****\n\n");
	}

	if (strncmp(algorithm,"opti",4) == 0) {
		printf("*** Room Booking - REJECTED / Optimal ****\n\n");
		fprintf(fp,"*** Room Booking - REJECTED / Optimal ****\n\n");
	}

	if (strncmp(algorithm,"all",3) == 0) {
		printf("*** Room Booking - REJECTED / ALL ****\n\n");
		fprintf(fp,"*** Room Booking - REJECTED / ALL ****\n\n");
	}

	// print the result seperated by tenants
	for (t = 0; t < 5; t++) {            			
		char *tenantText = (char *)malloc(10);
		strcpy(tenantText,tenants[t]);
		tenantText++;
		tenantText[0] = toupper(tenantText[0]);

		// write to console
		printf("%s (there are %i bookings rejected)\n\n",tenantText, rejected_events[t]);						
		printf("Date         Start   End     Type          Device\n");
		printf("===========================================================================\n");

		// write to file
		fprintf(fp,"%s (there are %i bookings rejected)\n\n",tenantText, rejected_events[t]);						
		fprintf(fp,"Date         Start   End     Type          Device\n");
		fprintf(fp,"===========================================================================\n");
		
		for (day = 10; day < 17; day++) {
			for(hour = 0; hour < 24; hour++) {	
				for(j = 0; j < MAX_EVENT; j++) {
					// early terminating
					if (eventList[j].accepted_state == true || eventList[j].duration < 1.0) {
						// empty event or not accpeted event
						continue;
					}
					else {				
						// print the event
						if (strcmp(eventList[j].tenant,tenants[t]) == 0 && eventList[j].mtm.tm_mday == day && eventList[j].mtm.tm_hour == hour) {
							// bookDevice do not need room
							struct tm endTime = handleDateTime(eventList[j].mtm, eventList[j].duration);
							
							// write to console
							printf("%04d-%02d-%02d   %02d:%02d   %02d:%02d   %-12s  %s\n",
							eventList[j].mtm.tm_year,eventList[j].mtm.tm_mon,eventList[j].mtm.tm_mday,
							eventList[j].mtm.tm_hour,eventList[j].mtm.tm_min,
							endTime.tm_hour,endTime.tm_min,
							eventList[j].event,
							eventList[j].deviceA);

							
							// write to console
							fprintf(fp,"%04d-%02d-%02d   %02d:%02d   %02d:%02d   %-12s  %s\n",
							eventList[j].mtm.tm_year,eventList[j].mtm.tm_mon,eventList[j].mtm.tm_mday,
							eventList[j].mtm.tm_hour,eventList[j].mtm.tm_min,
							endTime.tm_hour,endTime.tm_min,
							eventList[j].event,
							eventList[j].deviceA);

							if(strcmp(eventList[j].deviceA, "*") != 0){
								printf("%53s\n",eventList[j].deviceB);		// write to console
								fprintf(fp,"%53s\n",eventList[j].deviceB);	// write to file
							}
						}
					}									
				}
			}
		}
		
		tenantText--;
		printf("\n");
	}
	// write to console
	printf("\n- End -\n");
	printf("===========================================================================\n");

	// write to file
	fprintf(fp,"\n- End -\n");
	fprintf(fp,"===========================================================================\n");

	// close the file
	fclose(fp);
}

void printSummary(int* time_fcfs, int* time_prio, int totalEvents, int invalidRequest) {
	FILE *fp = fopen("RBM_Report_G16.txt", "a");
	const float BASE = 7*24;
	/*
		for reference
		time_****[0] = roomA
		time_****[1] = roomB
		time_****[2] = roomC
		time_****[3] = webcam_FHD_1
		time_****[4] = webcam_FHD_2
		time_****[5] = webcam_UHD
		time_****[6] = monitor_50_1
		time_****[7] = monitor_50_2
		time_****[8] = monitor_75
		time_****[9] = projector_2k_1
		time_****[10] = projector_2k_2
		time_****[11] = projector_4k
		time_****[12] = screen_100_1
		time_****[13] = screen_100_2
		time_****[14] = screen_150
		time_****[15] = total accepted events for algorithm ****
	*/

	float accepted_fcfs = time_fcfs[15];
	float accepted_prio = time_prio[15];

	// print to console
	printf("*** Room Booking Manager - Summary Report ***\n\n");
	printf("Performance:\n");

	// FCFS
	printf("\n  For FCFS:\n");
	printf("    Total Number of Bookings Received: %i (100%%)\n",totalEvents);
	printf("          Number of Bookings Assigned: %.0f (%.2f%%)\n",accepted_fcfs,accepted_fcfs*100/totalEvents);
	printf("          Number of Bookings Rejected: %.0f (%.2f%%)\n",totalEvents-accepted_fcfs, (totalEvents-accepted_fcfs)*100/totalEvents);
	printf("\n    Utilization of Time Slot:\n");
	printf("        Room_A: - %.2f%%\n",time_fcfs[0]*100/BASE);
	printf("        Room_B: - %.2f%%\n",time_fcfs[1]*100/BASE);
	printf("        Room_C: - %.2f%%\n",time_fcfs[2]*100/BASE);
	printf("        webcam_FHD: - %.2f%%\n",(time_fcfs[3]*100/BASE + time_fcfs[4]*100/BASE)/2);	// 2 device, so add both and get avg
	printf("        webcam_UHD: - %.2f%%\n",time_fcfs[5]*100/BASE);
	printf("        monitor_50: - %.2f%%\n",(time_fcfs[6]*100/BASE + time_fcfs[7]*100/BASE)/2);	// 2 device, so add both and get avg
	printf("        monitor_75: - %.2f%%\n",time_fcfs[8]*100/BASE);
	printf("        projector_2k: - %.2f%%\n",(time_fcfs[9]*100/BASE + time_fcfs[10]*100/BASE)/2);	// 2 device, so add both and get avg
	printf("        projector_4k: - %.2f%%\n",time_fcfs[11]*100/BASE);
	printf("        screen_100: - %.2f%%\n",(time_fcfs[12]*100/BASE + time_fcfs[13]*100/BASE)/2);	// 2 device, so add both and get avg
	printf("        screen_150: - %.2f%%\n",time_fcfs[14]*100/BASE);	
	printf("\n    Invalid request(s) made: %i\n",invalidRequest);

	// PRIO
	printf("\n  For PRIO:\n");
	printf("    Total Number of Bookings Received: %i (100%%)\n",totalEvents);
	printf("          Number of Bookings Assigned: %.0f (%.2f%%)\n",accepted_prio,accepted_prio*100/totalEvents);
	printf("          Number of Bookings Rejected: %.0f (%.2f%%)\n",totalEvents-accepted_prio, (totalEvents-accepted_prio)*100/totalEvents);
	printf("\n    Utilization of Time Slot:\n");
	printf("        Room_A: - %.2f%%\n",time_prio[0]*100/BASE);
	printf("        Room_B: - %.2f%%\n",time_prio[1]*100/BASE);
	printf("        Room_C: - %.2f%%\n",time_prio[2]*100/BASE);
	printf("        webcam_FHD: - %.2f%%\n",(time_prio[3]*100/BASE + time_prio[4]*100/BASE)/2);	// 2 device, so add both and get avg
	printf("        webcam_UHD: - %.2f%%\n",time_prio[5]*100/BASE);
	printf("        monitor_50: - %.2f%%\n",(time_prio[6]*100/BASE + time_prio[7]*100/BASE)/2);	// 2 device, so add both and get avg
	printf("        monitor_75: - %.2f%%\n",time_prio[8]*100/BASE);
	printf("        projector_2k: - %.2f%%\n",(time_prio[9]*100/BASE + time_prio[10]*100/BASE)/2);	// 2 device, so add both and get avg
	printf("        projector_4k: - %.2f%%\n",time_prio[11]*100/BASE);
	printf("        screen_100: - %.2f%%\n",(time_prio[12]*100/BASE + time_prio[13]*100/BASE)/2);	// 2 device, so add both and get avg
	printf("        screen_150: - %.2f%%\n",time_prio[14]*100/BASE);	
	printf("\n    Invalid request(s) made: %i\n",invalidRequest);

	// print to file
	fprintf(fp,"*** Room Booking Manager - Summary Report ***\n\n");
	fprintf(fp,"Performance:\n");

	// FCFS
	fprintf(fp,"\n  For FCFS:\n");
	fprintf(fp,"    Total Number of Bookings Received: %i (100%%)\n",totalEvents);
	fprintf(fp,"          Number of Bookings Assigned: %.0f (%.2f%%)\n",accepted_fcfs,accepted_fcfs*100/totalEvents);
	fprintf(fp,"          Number of Bookings Rejected: %.0f (%.2f%%)\n",totalEvents-accepted_fcfs, (totalEvents-accepted_fcfs)*100/totalEvents);
	fprintf(fp,"\n    Utilization of Time Slot:\n");
	fprintf(fp,"        Room_A: - %.2f%%\n",time_fcfs[0]*100/BASE);
	fprintf(fp,"        Room_B: - %.2f%%\n",time_fcfs[1]*100/BASE);
	fprintf(fp,"        Room_C: - %.2f%%\n",time_fcfs[2]*100/BASE);
	fprintf(fp,"        webcam_FHD: - %.2f%%\n",(time_fcfs[3]*100/BASE + time_fcfs[4]*100/BASE)/2);	// 2 device, so add both and get avg
	fprintf(fp,"        webcam_UHD: - %.2f%%\n",time_fcfs[5]*100/BASE);
	fprintf(fp,"        monitor_50: - %.2f%%\n",(time_fcfs[6]*100/BASE + time_fcfs[7]*100/BASE)/2);	// 2 device, so add both and get avg
	fprintf(fp,"        monitor_75: - %.2f%%\n",time_fcfs[8]*100/BASE);
	fprintf(fp,"        projector_2k: - %.2f%%\n",(time_fcfs[9]*100/BASE + time_fcfs[10]*100/BASE)/2);	// 2 device, so add both and get avg
	fprintf(fp,"        projector_4k: - %.2f%%\n",time_fcfs[11]*100/BASE);
	fprintf(fp,"        screen_100: - %.2f%%\n",(time_fcfs[12]*100/BASE + time_fcfs[13]*100/BASE)/2);	// 2 device, so add both and get avg
	fprintf(fp,"        screen_150: - %.2f%%\n",time_fcfs[14]*100/BASE);	
	fprintf(fp,"\n    Invalid request(s) made: %i\n",invalidRequest);

	// PRIO
	fprintf(fp,"\n  For PRIO:\n");
	fprintf(fp,"    Total Number of Bookings Received: %i (100%%)\n",totalEvents);
	fprintf(fp,"          Number of Bookings Assigned: %.0f (%.2f%%)\n",accepted_prio,accepted_prio*100/totalEvents);
	fprintf(fp,"          Number of Bookings Rejected: %.0f (%.2f%%)\n",totalEvents-accepted_prio, (totalEvents-accepted_prio)*100/totalEvents);
	fprintf(fp,"\n    Utilization of Time Slot:\n");
	fprintf(fp,"        Room_A: - %.2f%%\n",time_prio[0]*100/BASE);
	fprintf(fp,"        Room_B: - %.2f%%\n",time_prio[1]*100/BASE);
	fprintf(fp,"        Room_C: - %.2f%%\n",time_prio[2]*100/BASE);
	fprintf(fp,"        webcam_FHD: - %.2f%%\n",(time_prio[3]*100/BASE + time_prio[4]*100/BASE)/2);	// 2 device, so add both and get avg
	fprintf(fp,"        webcam_UHD: - %.2f%%\n",time_prio[5]*100/BASE);
	fprintf(fp,"        monitor_50: - %.2f%%\n",(time_prio[6]*100/BASE + time_prio[7]*100/BASE)/2);	// 2 device, so add both and get avg
	fprintf(fp,"        monitor_75: - %.2f%%\n",time_prio[8]*100/BASE);
	fprintf(fp,"        projector_2k: - %.2f%%\n",(time_prio[9]*100/BASE + time_prio[10]*100/BASE)/2);	// 2 device, so add both and get avg
	fprintf(fp,"        projector_4k: - %.2f%%\n",time_prio[11]*100/BASE);
	fprintf(fp,"        screen_100: - %.2f%%\n",(time_prio[12]*100/BASE + time_prio[13]*100/BASE)/2);	// 2 device, so add both and get avg
	fprintf(fp,"        screen_150: - %.2f%%\n",time_prio[14]*100/BASE);	
	fprintf(fp,"\n    Invalid request(s) made: %i\n",invalidRequest);

	// close the file
	fclose(fp);
}

int* printFCFS(struct Event eventList[MAX_EVENT], int total_event) {
	printf("Command is printBookings, print result of FCFS.\n");

	// create a time slot for algorithms to check with, all initialized with -1
	struct time_slot t_slot = {
		.roomA = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
	};
	memcpy ( &t_slot.roomB, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.roomC, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.webcam_FHD_1, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.webcam_FHD_2, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.webcam_UHD, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.monitor_50_1, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.monitor_50_2, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.monitor_75, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.projector_2k_1, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.projector_2k_2, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.projector_4k, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.screen_100_1, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.screen_100_2, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.screen_150, &t_slot.roomA, sizeof(t_slot.roomA) );
	
	int accpeted_events_of_day = 0;
	int accpeted_events_sum = 0;
	int i,j;

	// create pipes for each day for sending accepted event list
	int fd[7][2];

	// create pipes for each day for sending time slot
	int fd_timeSlot[7][2];

	
	for(i = 0; i < 7; i++){
		if(pipe(&fd[i][0])<0){
			printf("Pipe creation error\n");
			exit(1);
		}
		if(pipe(&fd_timeSlot[i][0])<0){
			printf("Pipe creation error\n");
			exit(1);
		}
	}
	
	// text variables
	char str_room_A[] = "room_A";
	char str_room_B[] = "room_B";
	char str_room_C[] = "room_C";
	char str_webcam_FHD[] = "webcam_FHD";
	char str_webcam_UHD[] = "webcam_UHD";
	char str_monitor_50[] = "monitor_50";
	char str_monitor_75[] = "monitor_75";
	char str_projector_2K[] = "projector_2K";
	char str_projector_4K[] = "projector_4K";
	char str_screen_100[] = "screen_100";
	char str_screen_150[] = "screen_150";

	// create forks for each day
	int pid;
	int accepted[MAX_EVENT];
	for (i = 0; i < 7; i++) {
		// child for processing
		if((pid = fork())== 0) {
			printf("PROCESS DATE is 1%i/5\n", i);
			printf("child %i, My PID: %d\n", i, (int) getpid());
			
			int date = i+10;
			int accepted_events = 0;

			struct Event accepted_eventList[MAX_EVENT];	// event list to store all accepted requests, then it will be sent back to parent			
			// ultilaztion list of all rooms and devices, + valid booking
			// 0~14: Rooms and devices, 15: total accepted booking
			int timeSlot_ultilization[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
			
			// apply scheduling to one day for each fork
			for (j = 0; j < total_event; j++) {

				// state of rooms and devices, initialized to true
				// if one of the room/device requested by the event is false, it will be rejected
				bool room_A = true;
				bool room_B = true;
				bool room_C = true;
				bool webcam_FHD_1 = true;
				bool webcam_FHD_2 = true;
				bool webcam_UHD = true;
				bool monitor_50_1 = true;
				bool monitor_50_2 = true;
				bool monitor_75 = true;
				bool projector_2k_1 = true;
				bool projector_2k_2 = true;
				bool projector_4k = true;
				bool screen_100_1 = true;
				bool screen_100_2 = true;
				bool screen_150 = true;	

				if(eventList[j].dateSlot == date) {		
					printf("date slot %i, date is a %i\n",eventList[j].dateSlot,date);												
					// only do day n of schdule
					// check if time slot is ocuppied
					// if yes, continue to next
					// if no, fill time slot with event number

					printf("+++++++ child %i, currently event %i\n",i,j);
					printf("------- child %i, current event has device %s and %s\n",i,eventList[j].deviceA,eventList[j].deviceB);


					if (eventList[j].duration == 0.0) {
						printf("child %i, breaked at event %i\n",i,j);
						break;
					}

					int time = eventList[j].timeSlot;		// get the time when the event starts
					int person = eventList[j].persons;		// get how many person is in the event
					int duration = eventList[j].duration;	// get the duration of the event, duration = how many slots it occupied
					bool isAccepted = true;				// if the time slot is blank, then it will be accepted, otherwise, it will be skipped
					char assigned_room;

					// only add* events will check the room usage
					if (person > 0 && person <= 20) {							
						int d;
						// reserve small room
						printf("child %i, checking rooms\n",i);
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration
							// check room A
							if (t_slot.roomA[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_A = false;
							}
							// check room B
							if (t_slot.roomB[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_B = false;
							}
							// check room C
							if (t_slot.roomC[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_C = false;
							}
						}

						// <= 10, can use room A or B, try not to use room C
						if (person <= 10) {
							if (room_A == false) {
								if (room_B == false) {
									if (room_C == false) {
										isAccepted = false;
										continue;
									}
									else { // give event room C
										printf("child %i, give room C\n",i);
										assigned_room = 'C';
										isAccepted = true;
														
										// time slot for testing
										// printTimeSlot(t_slot,i);
									}
								}
								else { // give event room B
									printf("child %i, give room B\n",i);
									assigned_room = 'B';
									isAccepted = true;
										
									// time slot for testing
									// printTimeSlot(t_slot,i);
								}
							}
							else { // give event room A
								printf("child %i, give room A\n",i);
								assigned_room = 'A';
								isAccepted = true;
								
								// time slot for testing
								// printTimeSlot(t_slot,i);
							}
						}
						// >10 person, must use room C
						else if (room_C == false) {
							isAccepted = false;
							continue;	// room C not available
						}
						else { // give event room C
							printf("child %i, give room C\n",i);
							assigned_room = 'C';
							isAccepted = true;

							// time slot for testing
							// printTimeSlot(t_slot,i);
						}
					}
					// else 0 people is in the bookDevice event, skip

					if ((strcmp(eventList[j].deviceA, str_webcam_FHD) == 0) || (strcmp(eventList[j].deviceB, str_webcam_FHD) == 0))
					{
						printf("child %i, webcam_FHD\n",i);
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on first webcam
							if (t_slot.webcam_FHD_1[time + d] >= 0) {
								webcam_FHD_1 = false;
							}
							if (t_slot.webcam_FHD_2[time + d] >= 0) {
								webcam_FHD_2 = false;
							}
						}

						// if 2 webcam_FHDs are requested at the same time
						if((strcmp(eventList[j].deviceA, str_webcam_FHD) == 0) && (strcmp(eventList[j].deviceB, str_webcam_FHD) == 0)) {
							if (webcam_FHD_1 == true && webcam_FHD_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 webcam_FHD is requested
						else {
							if (webcam_FHD_1 == false) {	
								if (webcam_FHD_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								webcam_FHD_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need webcam FHD
						webcam_FHD_1 = false;
						webcam_FHD_2 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_webcam_UHD) == 0) || (strcmp(eventList[j].deviceB, str_webcam_UHD) == 0))
					{
						printf("child %i, webcam_UHD\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.webcam_UHD[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								webcam_UHD = false;
							}
						}
						if (webcam_UHD == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {					
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need webcam uHD
						webcam_UHD = false;
					}

					if ((strcmp(eventList[j].deviceA, str_monitor_50) == 0) || (strcmp(eventList[j].deviceB, str_monitor_50) == 0))
					{		
						printf("child %i, monitor_50\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on the 2 monitors
							if (t_slot.monitor_50_1[time + d] >= 0) {
								monitor_50_1 = false;
							}
							if (t_slot.monitor_50_2[time + d] >= 0) {
								monitor_50_2 = false;
							}
						}

						// if 2 monitor_50s are requested at the same time
						if((strcmp(eventList[j].deviceA, str_monitor_50) == 0) && (strcmp(eventList[j].deviceB, str_monitor_50) == 0)) {
							if (monitor_50_1 == true && monitor_50_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 monitor_50 is requested
						else {
							if (monitor_50_1 == false) {	
								if (monitor_50_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								monitor_50_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need monitor 50
						monitor_50_1 = false;
						monitor_50_2 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_monitor_75) == 0) || (strcmp(eventList[j].deviceB, str_monitor_75) == 0))
					{
						printf("child %i, monitor_75\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.monitor_75[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								monitor_75 = false;
							}
						}
						if (monitor_75 == false) {
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							for(d = 0; d < duration; d++) {
								t_slot.monitor_75[time + d] = j;
							}
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need monitor 75
						monitor_75 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_projector_2K) == 0) || (strcmp(eventList[j].deviceB, str_projector_2K) == 0))
					{		
						printf("child %i, projector_2K\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on projector_2k 1
							if (t_slot.projector_2k_1[time + d] >= 0) {
								projector_2k_1 = false;
							}
							if (t_slot.projector_2k_2[time + d] >= 0) {
								projector_2k_2 = false;
							}
						}

						// if 2 projector_2Ks are requested at the same time
						if((strcmp(eventList[j].deviceA, str_projector_2K) == 0) && (strcmp(eventList[j].deviceB, str_projector_2K) == 0)) {
							if (projector_2k_1 == true && projector_2k_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 projector_2K is requested
						else {
							if (projector_2k_1 == false) {	
								if (projector_2k_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								projector_2k_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need projector 2k
						projector_2k_1 = false;
						projector_2k_2 = false;
					}
					

					if ((strcmp(eventList[j].deviceA, str_projector_4K) == 0) || (strcmp(eventList[j].deviceB, str_projector_4K) == 0))
					{		
						printf("child %i, projector_4K\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;
						bool isOccupied = false;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.projector_4k[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								projector_4k = false;
							}
						}
						if (projector_4k == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							isAccepted = true;
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);				
					}
					else {
						// no need projector 4k
						projector_4k = false;
					}

					// printf("strcmp for event %i is %i or %i\n", eventList[j].event_num, strcmp(eventList[j].deviceA, str_screen_100),strcmp(eventList[j].deviceB, str_screen_100));

					if ((strcmp(eventList[j].deviceA, str_screen_100) == 0) || (strcmp(eventList[j].deviceB, str_screen_100) == 0))
					{			
						printf("child %i, screen_100\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on screen_100 1/2
							if (t_slot.screen_100_1[time + d] >= 0) {
								screen_100_1 = false;
							}
							if (t_slot.screen_100_2[time + d] >= 0) {
								screen_100_2 = false;
							}
						}
						
						// if 2 screen_100s are requested at the same time
						if((strcmp(eventList[j].deviceA, str_screen_100) == 0) && (strcmp(eventList[j].deviceB, str_screen_100) == 0)) {
							if (screen_100_1 == true && screen_100_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 screen_100 is requested
						else {
							if (screen_100_1 == false) {	
								if (screen_100_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								screen_100_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);			
					}
					else {
						// no need screen 100
						screen_100_1 = false;
						screen_100_2 = false;
					}
					
					if ((strcmp(eventList[j].deviceA, str_screen_150) == 0) || (strcmp(eventList[j].deviceB, str_screen_150) == 0))
					{		
						printf("child %i, screen_150\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.screen_150[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								screen_150 = false;
							}
						}
						if (screen_150 == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							isAccepted = true;
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);						
					}
					else {
						// no need screen 150
						screen_150 = false;
					}

					// all the requested room and device are available, proceed to add to accpeted event list and write on the time slot
					if (isAccepted == true) {
						printf("accepted event %i\n", j);
						// copy the event to the accepted event list, then modify the accepted state and assigned room
						accepted_eventList[accepted_events] = eventList[j];	// copy the current event to a new list
						accepted_eventList[accepted_events].accepted_state = true;
						if (assigned_room) {
							accepted_eventList[accepted_events].room = assigned_room;	// assign room to the event									
						}
						accepted_events++;

						// fill the time slot for later checking
						int d;

						// fill room time slot, only one room will be filled
						switch (assigned_room)
						{
						case 'A': {
							for (d = 0; d < duration; d++) {
								t_slot.roomA[time + d] = j;
							}
							break;
						}
						case 'B': {
							for (d = 0; d < duration; d++) {
								t_slot.roomB[time + d] = j;
							}
							break;
						}
						case 'C': {
							for (d = 0; d < duration; d++) {
								t_slot.roomC[time + d] = j;
							}
							break;
						}								
						default:
							break;
						}

						// fill webcam_FHD time slot, only one of the webcam will be filled
						if (webcam_FHD_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.webcam_FHD_1[time + d] = j;
							}
						}
						if (webcam_FHD_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.webcam_FHD_2[time + d] = j;
							}
						}

						// fill webcam_UHD time slot
						if (webcam_UHD == true) {
							for(d = 0; d < duration; d++) {
								t_slot.webcam_UHD[time + d] = j;
							}
						}

						// fill monitor_50 time slot, only one of the monitors will be filled
						if (monitor_50_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_50_1[time + d] = j;
							}
						}
						if (monitor_50_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_50_2[time + d] = j;
							}
						}

						// fill monitor_75 time slot
						if (monitor_75 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_75[time + d] = j;
							}
						}

						// fill projector_2K time slot, only one of the projectors will be filled
						if (projector_2k_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_2k_1[time + d] = j;
							}
						}
						if (projector_2k_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_2k_2[time + d] = j;
							}
						}

						// fill projector_4K time slot
						if (projector_4k == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_4k[time + d] = j;
							}
						}

						// fill screen_100 time slot, only one of the screens will be filled
						if (screen_100_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_100_1[time + d] = j;
							}
						}
						if (screen_100_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_100_2[time + d] = j;
							}
						}

						// fill screen_150 time slot
						if (screen_150 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_150[time + d] = j;
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					// if not accepted, it will be skipped
				}
				
			}


			// sending accepted event list to parent
			printf("child %i, finished checking\n\n",i);
			close(fd[i][0]);
			if (accepted_events > 0) {	// if there is accepted events					
				printf("%i sending %i events\n", i, accepted_events);
				write(fd[i][1], accepted_eventList, sizeof(*eventList)*MAX_EVENT);	// tell parent the accepted event number list
			}
			else {	// if there is no accepted events
				//initialie event data	(date, time, duration, persons, initialize state is false)				
				printf("sending blank event\n\n");				
				struct Event blank = {
					.duration = -1.0,
				};
				write(fd[i][1], &blank, sizeof(*eventList)*MAX_EVENT);	// no event is accepted that day, return a blank event to parent
			}
			close(fd[i][1]);			

			printf("child %i, start combining time slot count\n",i);
			// ultilization of slots of all devices/room
			int h;
			for (h = 0; h < 24; h++) {				
				if (t_slot.roomA[h] >= 0) { timeSlot_ultilization[0]++; }
				if (t_slot.roomB[h] >= 0) { timeSlot_ultilization[1]++; }
				if (t_slot.roomC[h] >= 0) { timeSlot_ultilization[2]++; }
				if (t_slot.webcam_FHD_1[h] >= 0) { timeSlot_ultilization[3]++; }
				if (t_slot.webcam_FHD_2[h] >= 0) { timeSlot_ultilization[4]++; }
				if (t_slot.webcam_UHD[h] >= 0) { timeSlot_ultilization[5]++; }
				if (t_slot.monitor_50_1[h] >= 0) { timeSlot_ultilization[6]++; }
				if (t_slot.monitor_50_2[h] >= 0) { timeSlot_ultilization[7]++; }
				if (t_slot.monitor_75[h] >= 0) { timeSlot_ultilization[8]++; }
				if (t_slot.projector_2k_1[h] >= 0) { timeSlot_ultilization[9]++; }
				if (t_slot.projector_2k_2[h] >= 0) { timeSlot_ultilization[10]++; }
				if (t_slot.projector_4k[h] >= 0) { timeSlot_ultilization[11]++; }
				if (t_slot.screen_100_1[h] >= 0) { timeSlot_ultilization[12]++; }
				if (t_slot.screen_100_2[h] >= 0) { timeSlot_ultilization[13]++; }
				if (t_slot.screen_150[h] >= 0) { timeSlot_ultilization[14]++; }
			}
			// total accepted bookings
			timeSlot_ultilization[15] += accepted_events;

			// sending ultilization of time slots to parent
			printf("child %i, sending ultilization of time slots\n\n",i);

			close(fd_timeSlot[i][0]);
			write(fd_timeSlot[i][1], timeSlot_ultilization, sizeof(*timeSlot_ultilization)*16);	// tell parent the accepted event number list
			close(fd_timeSlot[i][1]);

			exit(0);
		}
		 //sleep(1);
	}
	

	int *full_timeSlot_ultilization = malloc(sizeof(int)*16);
	int ts;
	for (ts = 0; ts < 16; ts++) {
		*(full_timeSlot_ultilization+ts) = 0;
	}

	// collect child
	printf("collecting pipe\n");
	for (i = 0; i < 7; i++) {				
		wait(NULL);
		close(fd[i][1]);
		struct Event accepted_arr[MAX_EVENT];
		int m;
		int timeSlot_ultilization[16];

		// read the array that child sent if there is accepted events
		while(1) {
			// sleep(1);
			m = read(fd[i][0], accepted_arr, sizeof(*eventList)*MAX_EVENT);
			// printf("<<<<<<<<<   m is %i, first duration is %.1f\n",m,accepted_arr[0].duration);
			if (m >= 0) {
				printf("child %i returned array, size %i\n",i,m);
				break;
			}
		}

		// printf("\nsee what child %i send back\n",i);
		// for(m = 0; m < MAX_EVENT; m++) {
		// 	if (accepted_arr[m].duration > 0.0)
		// 		printEvent(accepted_arr[m],m);
		// }

		// for test print
		printf("child %i, start inserting back to returned array\n",i);
		int c;
		for (c = 0; c < MAX_EVENT; c++) {
			// early teminating, duration cannot be 0, skip to next date
			// printf("the event is %s\n",&accepted_arr[c].event);
			if (accepted_arr[c].duration < 1.0 || accepted_arr[c].accepted_state == false || accepted_arr[c].validaty == false) {
				printf("date 1%i/5, early terminate, no more output\n",i);
				break;
			}
			// print the accepted event					
			printf("replacing event %i with info from event %i\n",accepted_arr[c].event_num,c);
			int replacing_event_num = accepted_arr[c].event_num;

			// writing room
			eventList[replacing_event_num].room = accepted_arr[c].room;
			// printf("[[[[[[[[[ accepted is %c, writing %c to event %i\n",accepted_arr[c].room,eventList[replacing_event_num].room,replacing_event_num);
			
			eventList[replacing_event_num].accepted_state = accepted_arr[c].accepted_state;
			
			// printEvent(eventList[replacing_event_num],replacing_event_num);
		}
				
		printf("child %i, getting time slot array\n",i);
		// read the time slot ultilization array that child sent
		while(1) {
			// sleep(1);
			m = read(fd_timeSlot[i][0], timeSlot_ultilization, sizeof(*timeSlot_ultilization)*16);
			// printf("<<<<<<<<<   m is %i, first duration is %.1f\n",m,accepted_arr[0].duration);
			if (m >= 0) {
				printf("child %i returned time slot array, size %i\n",i,m);
				break;
			}
		}
		
		printf("child %i, start inserting back to returned array\n",i);
		for (c = 0; c < 16; c++) {
			full_timeSlot_ultilization[c] += timeSlot_ultilization[c];
		}

		printf("\n");
		close(fd[i][0]);
		close(fd_timeSlot[i][0]);
		// free(accepted_arr);
		// sleep(1);
	}

	// print the result based on the algorithm				
	printByTenants(eventList,"fcfs",total_event);

	return full_timeSlot_ultilization;
}

int* printPRIO(struct Event eventList[MAX_EVENT], int total_event) {
	printf("Command is printBookings, print result of PRIO.\n");

	// create a time slot for algorithms to check with, all initialized with -1
	struct time_slot t_slot = {
		.roomA = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
	};
	memcpy ( &t_slot.roomB, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.roomC, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.webcam_FHD_1, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.webcam_FHD_2, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.webcam_UHD, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.monitor_50_1, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.monitor_50_2, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.monitor_75, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.projector_2k_1, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.projector_2k_2, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.projector_4k, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.screen_100_1, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.screen_100_2, &t_slot.roomA, sizeof(t_slot.roomA) );
	memcpy ( &t_slot.screen_150, &t_slot.roomA, sizeof(t_slot.roomA) );
	
	int accpeted_events_of_day = 0;
	int accpeted_events_sum = 0;
	int i,j;

	// create pipes for each day for sending accepted event list
	int fd[7][2];

	// create pipes for each day for sending time slot
	int fd_timeSlot[7][2];
	
	for(i = 0; i < 7; i++){
		if(pipe(&fd[i][0])<0){
			printf("Pipe creation error\n");
			exit(1);
		}
		if(pipe(&fd_timeSlot[i][0])<0){
			printf("Pipe creation error\n");
			exit(1);
		}
	}
	
	// text variables
	char str_room_A[] = "room_A";
	char str_room_B[] = "room_B";
	char str_room_C[] = "room_C";
	char str_webcam_FHD[] = "webcam_FHD";
	char str_webcam_UHD[] = "webcam_UHD";
	char str_monitor_50[] = "monitor_50";
	char str_monitor_75[] = "monitor_75";
	char str_projector_2K[] = "projector_2K";
	char str_projector_4K[] = "projector_4K";
	char str_screen_100[] = "screen_100";
	char str_screen_150[] = "screen_150";
	char str_conference[] = "Conference";
	char str_presentation[] = "Presentation";
	char str_meeting[] = "Meeting";
	char str_device[] = "bookDevice";

	// create forks for each day
	int pid;
	int accepted[MAX_EVENT];
	for (i = 0; i < 7; i++) {
		// child for processing
		if((pid = fork())== 0) {
			printf("PROCESS DATE is 1%i/5\n", i);
			printf("child %i, My PID: %d\n", i, (int) getpid());
			
			int date = i+10;
			int accepted_events = 0;

			struct Event accepted_eventList[MAX_EVENT];	// event list to store all accepted requests, then it will be sent back to parent		
			// ultilaztion list of all rooms and devices, + valid booking
			// 0~14: Rooms and devices, 15: total accepted booking
			int timeSlot_ultilization[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
			
			// apply scheduling to one day for each fork
			for (j = 0; j < total_event; j++) {
				
				if(strcmp(eventList[j].event, str_conference) == 0){
					
				
				// state of rooms and devices, initialized to true
				// if one of the room/device requested by the event is false, it will be rejected
				bool room_A = true;
				bool room_B = true;
				bool room_C = true;
				bool webcam_FHD_1 = true;
				bool webcam_FHD_2 = true;
				bool webcam_UHD = true;
				bool monitor_50_1 = true;
				bool monitor_50_2 = true;
				bool monitor_75 = true;
				bool projector_2k_1 = true;
				bool projector_2k_2 = true;
				bool projector_4k = true;
				bool screen_100_1 = true;
				bool screen_100_2 = true;
				bool screen_150 = true;	

				if(eventList[j].dateSlot == date) {		
					printf("date slot %i, date is a %i\n",eventList[j].dateSlot,date);												
					// only do day n of schdule
					// check if time slot is ocuppied
					// if yes, continue to next
					// if no, fill time slot with event number

					printf("+++++++ child %i, currently event %i\n",i,j);
					printf("------- child %i, current event has device %s and %s\n",i,eventList[j].deviceA,eventList[j].deviceB);


					if (eventList[j].duration == 0.0) {
						printf("child %i, breaked at event %i\n",i,j);
						break;
					}

					int time = eventList[j].timeSlot;		// get the time when the event starts
					int person = eventList[j].persons;		// get how many person is in the event
					int duration = eventList[j].duration;	// get the duration of the event, duration = how many slots it occupied
					bool isAccepted = true;				// if the time slot is blank, then it will be accepted, otherwise, it will be skipped
					char assigned_room;

					// only add* events will check the room usage
					if (person > 0 && person <= 20) {							
						int d;
						// reserve small room
						printf("child %i, checking rooms\n",i);
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration
							// check room A
							if (t_slot.roomA[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_A = false;
							}
							// check room B
							if (t_slot.roomB[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_B = false;
							}
							// check room C
							if (t_slot.roomC[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_C = false;
							}
						}

						// <= 10, can use room A or B, try not to use room C
						if (person <= 10) {
							if (room_A == false) {
								if (room_B == false) {
									if (room_C == false) {
										isAccepted = false;
										continue;
									}
									else { // give event room C
										printf("child %i, give room C\n",i);
										assigned_room = 'C';
										isAccepted = true;
														
										// time slot for testing
										// printTimeSlot(t_slot,i);
									}
								}
								else { // give event room B
									printf("child %i, give room B\n",i);
									assigned_room = 'B';
									isAccepted = true;
										
									// time slot for testing
									// printTimeSlot(t_slot,i);
								}
							}
							else { // give event room A
								printf("child %i, give room A\n",i);
								assigned_room = 'A';
								isAccepted = true;
								
								// time slot for testing
								// printTimeSlot(t_slot,i);
							}
						}
						// >10 person, must use room C
						else if (room_C == false) {
							isAccepted = false;
							continue;	// room C not available
						}
						else { // give event room C
							printf("child %i, give room C\n",i);
							assigned_room = 'C';
							isAccepted = true;

							// time slot for testing
							// printTimeSlot(t_slot,i);
						}
					}
					// else 0 people is in the bookDevice event, skip

					if ((strcmp(eventList[j].deviceA, str_webcam_FHD) == 0) || (strcmp(eventList[j].deviceB, str_webcam_FHD) == 0))
					{
						printf("child %i, webcam_FHD\n",i);
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on first webcam
							if (t_slot.webcam_FHD_1[time + d] >= 0) {
								webcam_FHD_1 = false;
							}
							if (t_slot.webcam_FHD_2[time + d] >= 0) {
								webcam_FHD_2 = false;
							}
						}

						// if 2 webcam_FHDs are requested at the same time
						if((strcmp(eventList[j].deviceA, str_webcam_FHD) == 0) && (strcmp(eventList[j].deviceB, str_webcam_FHD) == 0)) {
							if (webcam_FHD_1 == true && webcam_FHD_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 webcam_FHD is requested
						else {
							if (webcam_FHD_1 == false) {	
								if (webcam_FHD_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								webcam_FHD_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need webcam FHD
						webcam_FHD_1 = false;
						webcam_FHD_2 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_webcam_UHD) == 0) || (strcmp(eventList[j].deviceB, str_webcam_UHD) == 0))
					{
						printf("child %i, webcam_UHD\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.webcam_UHD[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								webcam_UHD = false;
							}
						}
						if (webcam_UHD == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {					
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need webcam uHD
						webcam_UHD = false;
					}

					if ((strcmp(eventList[j].deviceA, str_monitor_50) == 0) || (strcmp(eventList[j].deviceB, str_monitor_50) == 0))
					{		
						printf("child %i, monitor_50\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on the 2 monitors
							if (t_slot.monitor_50_1[time + d] >= 0) {
								monitor_50_1 = false;
							}
							if (t_slot.monitor_50_2[time + d] >= 0) {
								monitor_50_2 = false;
							}
						}

						// if 2 monitor_50s are requested at the same time
						if((strcmp(eventList[j].deviceA, str_monitor_50) == 0) && (strcmp(eventList[j].deviceB, str_monitor_50) == 0)) {
							if (monitor_50_1 == true && monitor_50_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 monitor_50 is requested
						else {
							if (monitor_50_1 == false) {	
								if (monitor_50_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								monitor_50_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need monitor 50
						monitor_50_1 = false;
						monitor_50_2 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_monitor_75) == 0) || (strcmp(eventList[j].deviceB, str_monitor_75) == 0))
					{
						printf("child %i, monitor_75\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.monitor_75[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								monitor_75 = false;
							}
						}
						if (monitor_75 == false) {
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							for(d = 0; d < duration; d++) {
								t_slot.monitor_75[time + d] = j;
							}
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need monitor 75
						monitor_75 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_projector_2K) == 0) || (strcmp(eventList[j].deviceB, str_projector_2K) == 0))
					{		
						printf("child %i, projector_2K\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on projector_2k 1
							if (t_slot.projector_2k_1[time + d] >= 0) {
								projector_2k_1 = false;
							}
							if (t_slot.projector_2k_2[time + d] >= 0) {
								projector_2k_2 = false;
							}
						}

						// if 2 projector_2Ks are requested at the same time
						if((strcmp(eventList[j].deviceA, str_projector_2K) == 0) && (strcmp(eventList[j].deviceB, str_projector_2K) == 0)) {
							if (projector_2k_1 == true && projector_2k_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 projector_2K is requested
						else {
							if (projector_2k_1 == false) {	
								if (projector_2k_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								projector_2k_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need projector 2k
						projector_2k_1 = false;
						projector_2k_2 = false;
					}
					

					if ((strcmp(eventList[j].deviceA, str_projector_4K) == 0) || (strcmp(eventList[j].deviceB, str_projector_4K) == 0))
					{		
						printf("child %i, projector_4K\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;
						bool isOccupied = false;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.projector_4k[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								projector_4k = false;
							}
						}
						if (projector_4k == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							isAccepted = true;
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);				
					}
					else {
						// no need projector 4k
						projector_4k = false;
					}

					// printf("strcmp for event %i is %i or %i\n", eventList[j].event_num, strcmp(eventList[j].deviceA, str_screen_100),strcmp(eventList[j].deviceB, str_screen_100));

					if ((strcmp(eventList[j].deviceA, str_screen_100) == 0) || (strcmp(eventList[j].deviceB, str_screen_100) == 0))
					{			
						printf("child %i, screen_100\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on screen_100 1/2
							if (t_slot.screen_100_1[time + d] >= 0) {
								screen_100_1 = false;
							}
							if (t_slot.screen_100_2[time + d] >= 0) {
								screen_100_2 = false;
							}
						}
						
						// if 2 screen_100s are requested at the same time
						if((strcmp(eventList[j].deviceA, str_screen_100) == 0) && (strcmp(eventList[j].deviceB, str_screen_100) == 0)) {
							if (screen_100_1 == true && screen_100_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 screen_100 is requested
						else {
							if (screen_100_1 == false) {	
								if (screen_100_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								screen_100_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);			
					}
					else {
						// no need screen 100
						screen_100_1 = false;
						screen_100_2 = false;
					}
					
					if ((strcmp(eventList[j].deviceA, str_screen_150) == 0) || (strcmp(eventList[j].deviceB, str_screen_150) == 0))
					{		
						printf("child %i, screen_150\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.screen_150[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								screen_150 = false;
							}
						}
						if (screen_150 == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							isAccepted = true;
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);						
					}
					else {
						// no need screen 150
						screen_150 = false;
					}

					// all the requested room and device are available, proceed to add to accpeted event list and write on the time slot
					if (isAccepted == true) {
						printf("accepted event %i\n", j);
						// copy the event to the accepted event list, then modify the accepted state and assigned room
						accepted_eventList[accepted_events] = eventList[j];	// copy the current event to a new list
						accepted_eventList[accepted_events].accepted_state = true;
						if (assigned_room) {
							accepted_eventList[accepted_events].room = assigned_room;	// assign room to the event									
						}
						accepted_events++;

						// fill the time slot for later checking
						int d;

						// fill room time slot, only one room will be filled
						switch (assigned_room)
						{
						case 'A': {
							for (d = 0; d < duration; d++) {
								t_slot.roomA[time + d] = j;
							}
							break;
						}
						case 'B': {
							for (d = 0; d < duration; d++) {
								t_slot.roomB[time + d] = j;
							}
							break;
						}
						case 'C': {
							for (d = 0; d < duration; d++) {
								t_slot.roomC[time + d] = j;
							}
							break;
						}								
						default:
							break;
						}

						// fill webcam_FHD time slot, only one of the webcam will be filled
						if (webcam_FHD_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.webcam_FHD_1[time + d] = j;
							}
						}
						if (webcam_FHD_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.webcam_FHD_2[time + d] = j;
							}
						}

						// fill webcam_UHD time slot
						if (webcam_UHD == true) {
							for(d = 0; d < duration; d++) {
								t_slot.webcam_UHD[time + d] = j;
							}
						}

						// fill monitor_50 time slot, only one of the monitors will be filled
						if (monitor_50_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_50_1[time + d] = j;
							}
						}
						if (monitor_50_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_50_2[time + d] = j;
							}
						}

						// fill monitor_75 time slot
						if (monitor_75 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_75[time + d] = j;
							}
						}

						// fill projector_2K time slot, only one of the projectors will be filled
						if (projector_2k_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_2k_1[time + d] = j;
							}
						}
						if (projector_2k_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_2k_2[time + d] = j;
							}
						}

						// fill projector_4K time slot
						if (projector_4k == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_4k[time + d] = j;
							}
						}

						// fill screen_100 time slot, only one of the screens will be filled
						if (screen_100_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_100_1[time + d] = j;
							}
						}
						if (screen_100_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_100_2[time + d] = j;
							}
						}

						// fill screen_150 time slot
						if (screen_150 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_150[time + d] = j;
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					// if not accepted, it will be skipped
				}
				
			 	}
			 	else
			 		continue;
			}

						
						
			for (j = 0; j < total_event; j++) {
				
				if(strcmp(eventList[j].event, str_presentation) == 0){
					
				
				// state of rooms and devices, initialized to true
				// if one of the room/device requested by the event is false, it will be rejected
				bool room_A = true;
				bool room_B = true;
				bool room_C = true;
				bool webcam_FHD_1 = true;
				bool webcam_FHD_2 = true;
				bool webcam_UHD = true;
				bool monitor_50_1 = true;
				bool monitor_50_2 = true;
				bool monitor_75 = true;
				bool projector_2k_1 = true;
				bool projector_2k_2 = true;
				bool projector_4k = true;
				bool screen_100_1 = true;
				bool screen_100_2 = true;
				bool screen_150 = true;	

				if(eventList[j].dateSlot == date) {		
					printf("date slot %i, date is a %i\n",eventList[j].dateSlot,date);												
					// only do day n of schdule
					// check if time slot is ocuppied
					// if yes, continue to next
					// if no, fill time slot with event number

					printf("+++++++ child %i, currently event %i\n",i,j);
					printf("------- child %i, current event has device %s and %s\n",i,eventList[j].deviceA,eventList[j].deviceB);


					if (eventList[j].duration == 0.0) {
						printf("child %i, breaked at event %i\n",i,j);
						break;
					}

					int time = eventList[j].timeSlot;		// get the time when the event starts
					int person = eventList[j].persons;		// get how many person is in the event
					int duration = eventList[j].duration;	// get the duration of the event, duration = how many slots it occupied
					bool isAccepted = true;				// if the time slot is blank, then it will be accepted, otherwise, it will be skipped
					char assigned_room;

					// only add* events will check the room usage
					if (person > 0 && person <= 20) {							
						int d;
						// reserve small room
						printf("child %i, checking rooms\n",i);
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration
							// check room A
							if (t_slot.roomA[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_A = false;
							}
							// check room B
							if (t_slot.roomB[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_B = false;
							}
							// check room C
							if (t_slot.roomC[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_C = false;
							}
						}

						// <= 10, can use room A or B, try not to use room C
						if (person <= 10) {
							if (room_A == false) {
								if (room_B == false) {
									if (room_C == false) {
										isAccepted = false;
										continue;
									}
									else { // give event room C
										printf("child %i, give room C\n",i);
										assigned_room = 'C';
										isAccepted = true;
														
										// time slot for testing
										// printTimeSlot(t_slot,i);
									}
								}
								else { // give event room B
									printf("child %i, give room B\n",i);
									assigned_room = 'B';
									isAccepted = true;
										
									// time slot for testing
									// printTimeSlot(t_slot,i);
								}
							}
							else { // give event room A
								printf("child %i, give room A\n",i);
								assigned_room = 'A';
								isAccepted = true;
								
								// time slot for testing
								// printTimeSlot(t_slot,i);
							}
						}
						// >10 person, must use room C
						else if (room_C == false) {
							isAccepted = false;
							continue;	// room C not available
						}
						else { // give event room C
							printf("child %i, give room C\n",i);
							assigned_room = 'C';
							isAccepted = true;

							// time slot for testing
							// printTimeSlot(t_slot,i);
						}
					}
					// else 0 people is in the bookDevice event, skip

					if ((strcmp(eventList[j].deviceA, str_webcam_FHD) == 0) || (strcmp(eventList[j].deviceB, str_webcam_FHD) == 0))
					{
						printf("child %i, webcam_FHD\n",i);
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on first webcam
							if (t_slot.webcam_FHD_1[time + d] >= 0) {
								webcam_FHD_1 = false;
							}
							if (t_slot.webcam_FHD_2[time + d] >= 0) {
								webcam_FHD_2 = false;
							}
						}

						// if 2 webcam_FHDs are requested at the same time
						if((strcmp(eventList[j].deviceA, str_webcam_FHD) == 0) && (strcmp(eventList[j].deviceB, str_webcam_FHD) == 0)) {
							if (webcam_FHD_1 == true && webcam_FHD_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 webcam_FHD is requested
						else {
							if (webcam_FHD_1 == false) {	
								if (webcam_FHD_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								webcam_FHD_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need webcam FHD
						webcam_FHD_1 = false;
						webcam_FHD_2 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_webcam_UHD) == 0) || (strcmp(eventList[j].deviceB, str_webcam_UHD) == 0))
					{
						printf("child %i, webcam_UHD\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.webcam_UHD[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								webcam_UHD = false;
							}
						}
						if (webcam_UHD == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {					
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need webcam uHD
						webcam_UHD = false;
					}

					if ((strcmp(eventList[j].deviceA, str_monitor_50) == 0) || (strcmp(eventList[j].deviceB, str_monitor_50) == 0))
					{		
						printf("child %i, monitor_50\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on the 2 monitors
							if (t_slot.monitor_50_1[time + d] >= 0) {
								monitor_50_1 = false;
							}
							if (t_slot.monitor_50_2[time + d] >= 0) {
								monitor_50_2 = false;
							}
						}

						// if 2 monitor_50s are requested at the same time
						if((strcmp(eventList[j].deviceA, str_monitor_50) == 0) && (strcmp(eventList[j].deviceB, str_monitor_50) == 0)) {
							if (monitor_50_1 == true && monitor_50_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 monitor_50 is requested
						else {
							if (monitor_50_1 == false) {	
								if (monitor_50_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								monitor_50_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need monitor 50
						monitor_50_1 = false;
						monitor_50_2 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_monitor_75) == 0) || (strcmp(eventList[j].deviceB, str_monitor_75) == 0))
					{
						printf("child %i, monitor_75\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.monitor_75[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								monitor_75 = false;
							}
						}
						if (monitor_75 == false) {
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							for(d = 0; d < duration; d++) {
								t_slot.monitor_75[time + d] = j;
							}
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need monitor 75
						monitor_75 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_projector_2K) == 0) || (strcmp(eventList[j].deviceB, str_projector_2K) == 0))
					{		
						printf("child %i, projector_2K\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on projector_2k 1
							if (t_slot.projector_2k_1[time + d] >= 0) {
								projector_2k_1 = false;
							}
							if (t_slot.projector_2k_2[time + d] >= 0) {
								projector_2k_2 = false;
							}
						}

						// if 2 projector_2Ks are requested at the same time
						if((strcmp(eventList[j].deviceA, str_projector_2K) == 0) && (strcmp(eventList[j].deviceB, str_projector_2K) == 0)) {
							if (projector_2k_1 == true && projector_2k_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 projector_2K is requested
						else {
							if (projector_2k_1 == false) {	
								if (projector_2k_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								projector_2k_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need projector 2k
						projector_2k_1 = false;
						projector_2k_2 = false;
					}
					

					if ((strcmp(eventList[j].deviceA, str_projector_4K) == 0) || (strcmp(eventList[j].deviceB, str_projector_4K) == 0))
					{		
						printf("child %i, projector_4K\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;
						bool isOccupied = false;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.projector_4k[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								projector_4k = false;
							}
						}
						if (projector_4k == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							isAccepted = true;
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);				
					}
					else {
						// no need projector 4k
						projector_4k = false;
					}

					// printf("strcmp for event %i is %i or %i\n", eventList[j].event_num, strcmp(eventList[j].deviceA, str_screen_100),strcmp(eventList[j].deviceB, str_screen_100));

					if ((strcmp(eventList[j].deviceA, str_screen_100) == 0) || (strcmp(eventList[j].deviceB, str_screen_100) == 0))
					{			
						printf("child %i, screen_100\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on screen_100 1/2
							if (t_slot.screen_100_1[time + d] >= 0) {
								screen_100_1 = false;
							}
							if (t_slot.screen_100_2[time + d] >= 0) {
								screen_100_2 = false;
							}
						}
						
						// if 2 screen_100s are requested at the same time
						if((strcmp(eventList[j].deviceA, str_screen_100) == 0) && (strcmp(eventList[j].deviceB, str_screen_100) == 0)) {
							if (screen_100_1 == true && screen_100_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 screen_100 is requested
						else {
							if (screen_100_1 == false) {	
								if (screen_100_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								screen_100_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);			
					}
					else {
						// no need screen 100
						screen_100_1 = false;
						screen_100_2 = false;
					}
					
					if ((strcmp(eventList[j].deviceA, str_screen_150) == 0) || (strcmp(eventList[j].deviceB, str_screen_150) == 0))
					{		
						printf("child %i, screen_150\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.screen_150[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								screen_150 = false;
							}
						}
						if (screen_150 == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							isAccepted = true;
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);						
					}
					else {
						// no need screen 150
						screen_150 = false;
					}

					// all the requested room and device are available, proceed to add to accpeted event list and write on the time slot
					if (isAccepted == true) {
						printf("accepted event %i\n", j);
						// copy the event to the accepted event list, then modify the accepted state and assigned room
						accepted_eventList[accepted_events] = eventList[j];	// copy the current event to a new list
						accepted_eventList[accepted_events].accepted_state = true;
						if (assigned_room) {
							accepted_eventList[accepted_events].room = assigned_room;	// assign room to the event									
						}
						accepted_events++;

						// fill the time slot for later checking
						int d;

						// fill room time slot, only one room will be filled
						switch (assigned_room)
						{
						case 'A': {
							for (d = 0; d < duration; d++) {
								t_slot.roomA[time + d] = j;
							}
							break;
						}
						case 'B': {
							for (d = 0; d < duration; d++) {
								t_slot.roomB[time + d] = j;
							}
							break;
						}
						case 'C': {
							for (d = 0; d < duration; d++) {
								t_slot.roomC[time + d] = j;
							}
							break;
						}								
						default:
							break;
						}

						// fill webcam_FHD time slot, only one of the webcam will be filled
						if (webcam_FHD_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.webcam_FHD_1[time + d] = j;
							}
						}
						if (webcam_FHD_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.webcam_FHD_2[time + d] = j;
							}
						}

						// fill webcam_UHD time slot
						if (webcam_UHD == true) {
							for(d = 0; d < duration; d++) {
								t_slot.webcam_UHD[time + d] = j;
							}
						}

						// fill monitor_50 time slot, only one of the monitors will be filled
						if (monitor_50_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_50_1[time + d] = j;
							}
						}
						if (monitor_50_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_50_2[time + d] = j;
							}
						}

						// fill monitor_75 time slot
						if (monitor_75 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_75[time + d] = j;
							}
						}

						// fill projector_2K time slot, only one of the projectors will be filled
						if (projector_2k_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_2k_1[time + d] = j;
							}
						}
						if (projector_2k_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_2k_2[time + d] = j;
							}
						}

						// fill projector_4K time slot
						if (projector_4k == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_4k[time + d] = j;
							}
						}

						// fill screen_100 time slot, only one of the screens will be filled
						if (screen_100_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_100_1[time + d] = j;
							}
						}
						if (screen_100_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_100_2[time + d] = j;
							}
						}

						// fill screen_150 time slot
						if (screen_150 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_150[time + d] = j;
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					// if not accepted, it will be skipped
				}
				
			 	}
			 	else
			 		continue;
			}

			for (j = 0; j < total_event; j++) {
				
				if(strcmp(eventList[j].event, str_meeting) == 0){
					
				
				// state of rooms and devices, initialized to true
				// if one of the room/device requested by the event is false, it will be rejected
				bool room_A = true;
				bool room_B = true;
				bool room_C = true;
				bool webcam_FHD_1 = true;
				bool webcam_FHD_2 = true;
				bool webcam_UHD = true;
				bool monitor_50_1 = true;
				bool monitor_50_2 = true;
				bool monitor_75 = true;
				bool projector_2k_1 = true;
				bool projector_2k_2 = true;
				bool projector_4k = true;
				bool screen_100_1 = true;
				bool screen_100_2 = true;
				bool screen_150 = true;	

				if(eventList[j].dateSlot == date) {		
					printf("date slot %i, date is a %i\n",eventList[j].dateSlot,date);												
					// only do day n of schdule
					// check if time slot is ocuppied
					// if yes, continue to next
					// if no, fill time slot with event number

					printf("+++++++ child %i, currently event %i\n",i,j);
					printf("------- child %i, current event has device %s and %s\n",i,eventList[j].deviceA,eventList[j].deviceB);


					if (eventList[j].duration == 0.0) {
						printf("child %i, breaked at event %i\n",i,j);
						break;
					}

					int time = eventList[j].timeSlot;		// get the time when the event starts
					int person = eventList[j].persons;		// get how many person is in the event
					int duration = eventList[j].duration;	// get the duration of the event, duration = how many slots it occupied
					bool isAccepted = true;				// if the time slot is blank, then it will be accepted, otherwise, it will be skipped
					char assigned_room;

					// only add* events will check the room usage
					if (person > 0 && person <= 20) {							
						int d;
						// reserve small room
						printf("child %i, checking rooms\n",i);
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration
							// check room A
							if (t_slot.roomA[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_A = false;
							}
							// check room B
							if (t_slot.roomB[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_B = false;
							}
							// check room C
							if (t_slot.roomC[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_C = false;
							}
						}

						// <= 10, can use room A or B, try not to use room C
						if (person <= 10) {
							if (room_A == false) {
								if (room_B == false) {
									if (room_C == false) {
										isAccepted = false;
										continue;
									}
									else { // give event room C
										printf("child %i, give room C\n",i);
										assigned_room = 'C';
										isAccepted = true;
														
										// time slot for testing
										// printTimeSlot(t_slot,i);
									}
								}
								else { // give event room B
									printf("child %i, give room B\n",i);
									assigned_room = 'B';
									isAccepted = true;
										
									// time slot for testing
									// printTimeSlot(t_slot,i);
								}
							}
							else { // give event room A
								printf("child %i, give room A\n",i);
								assigned_room = 'A';
								isAccepted = true;
								
								// time slot for testing
								// printTimeSlot(t_slot,i);
							}
						}
						// >10 person, must use room C
						else if (room_C == false) {
							isAccepted = false;
							continue;	// room C not available
						}
						else { // give event room C
							printf("child %i, give room C\n",i);
							assigned_room = 'C';
							isAccepted = true;

							// time slot for testing
							// printTimeSlot(t_slot,i);
						}
					}
					// else 0 people is in the bookDevice event, skip

					if ((strcmp(eventList[j].deviceA, str_webcam_FHD) == 0) || (strcmp(eventList[j].deviceB, str_webcam_FHD) == 0))
					{
						printf("child %i, webcam_FHD\n",i);
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on first webcam
							if (t_slot.webcam_FHD_1[time + d] >= 0) {
								webcam_FHD_1 = false;
							}
							if (t_slot.webcam_FHD_2[time + d] >= 0) {
								webcam_FHD_2 = false;
							}
						}

						// if 2 webcam_FHDs are requested at the same time
						if((strcmp(eventList[j].deviceA, str_webcam_FHD) == 0) && (strcmp(eventList[j].deviceB, str_webcam_FHD) == 0)) {
							if (webcam_FHD_1 == true && webcam_FHD_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 webcam_FHD is requested
						else {
							if (webcam_FHD_1 == false) {	
								if (webcam_FHD_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								webcam_FHD_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need webcam FHD
						webcam_FHD_1 = false;
						webcam_FHD_2 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_webcam_UHD) == 0) || (strcmp(eventList[j].deviceB, str_webcam_UHD) == 0))
					{
						printf("child %i, webcam_UHD\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.webcam_UHD[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								webcam_UHD = false;
							}
						}
						if (webcam_UHD == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {					
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need webcam uHD
						webcam_UHD = false;
					}

					if ((strcmp(eventList[j].deviceA, str_monitor_50) == 0) || (strcmp(eventList[j].deviceB, str_monitor_50) == 0))
					{		
						printf("child %i, monitor_50\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on the 2 monitors
							if (t_slot.monitor_50_1[time + d] >= 0) {
								monitor_50_1 = false;
							}
							if (t_slot.monitor_50_2[time + d] >= 0) {
								monitor_50_2 = false;
							}
						}

						// if 2 monitor_50s are requested at the same time
						if((strcmp(eventList[j].deviceA, str_monitor_50) == 0) && (strcmp(eventList[j].deviceB, str_monitor_50) == 0)) {
							if (monitor_50_1 == true && monitor_50_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 monitor_50 is requested
						else {
							if (monitor_50_1 == false) {	
								if (monitor_50_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								monitor_50_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need monitor 50
						monitor_50_1 = false;
						monitor_50_2 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_monitor_75) == 0) || (strcmp(eventList[j].deviceB, str_monitor_75) == 0))
					{
						printf("child %i, monitor_75\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.monitor_75[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								monitor_75 = false;
							}
						}
						if (monitor_75 == false) {
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							for(d = 0; d < duration; d++) {
								t_slot.monitor_75[time + d] = j;
							}
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need monitor 75
						monitor_75 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_projector_2K) == 0) || (strcmp(eventList[j].deviceB, str_projector_2K) == 0))
					{		
						printf("child %i, projector_2K\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on projector_2k 1
							if (t_slot.projector_2k_1[time + d] >= 0) {
								projector_2k_1 = false;
							}
							if (t_slot.projector_2k_2[time + d] >= 0) {
								projector_2k_2 = false;
							}
						}

						// if 2 projector_2Ks are requested at the same time
						if((strcmp(eventList[j].deviceA, str_projector_2K) == 0) && (strcmp(eventList[j].deviceB, str_projector_2K) == 0)) {
							if (projector_2k_1 == true && projector_2k_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 projector_2K is requested
						else {
							if (projector_2k_1 == false) {	
								if (projector_2k_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								projector_2k_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need projector 2k
						projector_2k_1 = false;
						projector_2k_2 = false;
					}
					

					if ((strcmp(eventList[j].deviceA, str_projector_4K) == 0) || (strcmp(eventList[j].deviceB, str_projector_4K) == 0))
					{		
						printf("child %i, projector_4K\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;
						bool isOccupied = false;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.projector_4k[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								projector_4k = false;
							}
						}
						if (projector_4k == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							isAccepted = true;
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);				
					}
					else {
						// no need projector 4k
						projector_4k = false;
					}

					// printf("strcmp for event %i is %i or %i\n", eventList[j].event_num, strcmp(eventList[j].deviceA, str_screen_100),strcmp(eventList[j].deviceB, str_screen_100));

					if ((strcmp(eventList[j].deviceA, str_screen_100) == 0) || (strcmp(eventList[j].deviceB, str_screen_100) == 0))
					{			
						printf("child %i, screen_100\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on screen_100 1/2
							if (t_slot.screen_100_1[time + d] >= 0) {
								screen_100_1 = false;
							}
							if (t_slot.screen_100_2[time + d] >= 0) {
								screen_100_2 = false;
							}
						}
						
						// if 2 screen_100s are requested at the same time
						if((strcmp(eventList[j].deviceA, str_screen_100) == 0) && (strcmp(eventList[j].deviceB, str_screen_100) == 0)) {
							if (screen_100_1 == true && screen_100_2 == true) {
								isAccepted = true;
							}
							else {
								isAccepted = false;
							}
						}
						// only 1 screen_100 is requested
						else {
							if (screen_100_1 == false) {	
								if (screen_100_2 == false) {	// both webcam is busy, skip
									isAccepted = false;
									continue;
								}
								else {
									isAccepted = true;// webcam 2 is ok
								}
							}
							else {
								screen_100_2 = false;	// webcam 1 is ok, not use webcam 2
								isAccepted = true;	
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);			
					}
					else {
						// no need screen 100
						screen_100_1 = false;
						screen_100_2 = false;
					}
					
					if ((strcmp(eventList[j].deviceA, str_screen_150) == 0) || (strcmp(eventList[j].deviceB, str_screen_150) == 0))
					{		
						printf("child %i, screen_150\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.screen_150[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								screen_150 = false;
							}
						}
						if (screen_150 == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							isAccepted = true;
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);						
					}
					else {
						// no need screen 150
						screen_150 = false;
					}
					// all the requested room and device are available, proceed to add to accpeted event list and write on the time slot
					if (isAccepted == true) {
						printf("accepted event %i\n", j);
						// copy the event to the accepted event list, then modify the accepted state and assigned room
						accepted_eventList[accepted_events] = eventList[j];	// copy the current event to a new list
						accepted_eventList[accepted_events].accepted_state = true;
						if (assigned_room) {
							accepted_eventList[accepted_events].room = assigned_room;	// assign room to the event									
						}
						accepted_events++;

						// fill the time slot for later checking
						int d;

						// fill room time slot, only one room will be filled
						switch (assigned_room)
						{
						case 'A': {
							for (d = 0; d < duration; d++) {
								t_slot.roomA[time + d] = j;
							}
							break;
						}
						case 'B': {
							for (d = 0; d < duration; d++) {
								t_slot.roomB[time + d] = j;
							}
							break;
						}
						case 'C': {
							for (d = 0; d < duration; d++) {
								t_slot.roomC[time + d] = j;
							}
							break;
						}								
						default:
							break;
						}

						// fill webcam_FHD time slot, only one of the webcam will be filled
						if (webcam_FHD_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.webcam_FHD_1[time + d] = j;
							}
						}
						if (webcam_FHD_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.webcam_FHD_2[time + d] = j;
							}
						}

						// fill webcam_UHD time slot
						if (webcam_UHD == true) {
							for(d = 0; d < duration; d++) {
								t_slot.webcam_UHD[time + d] = j;
							}
						}

						// fill monitor_50 time slot, only one of the monitors will be filled
						if (monitor_50_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_50_1[time + d] = j;
							}
						}
						if (monitor_50_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_50_2[time + d] = j;
							}
						}

						// fill monitor_75 time slot
						if (monitor_75 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_75[time + d] = j;
							}
						}

						// fill projector_2K time slot, only one of the projectors will be filled
						if (projector_2k_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_2k_1[time + d] = j;
							}
						}
						if (projector_2k_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_2k_2[time + d] = j;
							}
						}

						// fill projector_4K time slot
						if (projector_4k == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_4k[time + d] = j;
							}
						}

						// fill screen_100 time slot, only one of the screens will be filled
						if (screen_100_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_100_1[time + d] = j;
							}
						}
						if (screen_100_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_100_2[time + d] = j;
							}
						}

						// fill screen_150 time slot
						if (screen_150 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_150[time + d] = j;
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					// if not accepted, it will be skipped
				}
				
			 	}
			 	else
			 		continue;
			}

			for (j = 0; j < total_event; j++) {
				
				if(strcmp(eventList[j].event, str_device) == 0){
					
				
				// state of rooms and devices, initialized to true
				// if one of the room/device requested by the event is false, it will be rejected
				bool room_A = true;
				bool room_B = true;
				bool room_C = true;
				bool webcam_FHD_1 = true;
				bool webcam_FHD_2 = true;
				bool webcam_UHD = true;
				bool monitor_50_1 = true;
				bool monitor_50_2 = true;
				bool monitor_75 = true;
				bool projector_2k_1 = true;
				bool projector_2k_2 = true;
				bool projector_4k = true;
				bool screen_100_1 = true;
				bool screen_100_2 = true;
				bool screen_150 = true;	

				if(eventList[j].dateSlot == date) {		
					printf("date slot %i, date is a %i\n",eventList[j].dateSlot,date);												
					// only do day n of schdule
					// check if time slot is ocuppied
					// if yes, continue to next
					// if no, fill time slot with event number

					printf("+++++++ child %i, currently event %i\n",i,j);
					printf("------- child %i, current event has device %s and %s\n",i,eventList[j].deviceA,eventList[j].deviceB);


					if (eventList[j].duration == 0.0) {
						printf("child %i, breaked at event %i\n",i,j);
						break;
					}

					int time = eventList[j].timeSlot;		// get the time when the event starts
					int person = eventList[j].persons;		// get how many person is in the event
					int duration = eventList[j].duration;	// get the duration of the event, duration = how many slots it occupied
					bool isAccepted = true;				// if the time slot is blank, then it will be accepted, otherwise, it will be skipped
					char assigned_room;

					// only add* events will check the room usage
					if (person > 0 && person <= 20) {							
						int d;
						// reserve small room
						printf("child %i, checking rooms\n",i);
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration
							// check room A
							if (t_slot.roomA[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_A = false;
							}
							// check room B
							if (t_slot.roomB[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_B = false;
							}
							// check room C
							if (t_slot.roomC[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								room_C = false;
							}
						}

						// <= 10, can use room A or B, try not to use room C
						if (person <= 10) {
							if (room_A == false) {
								if (room_B == false) {
									if (room_C == false) {
										isAccepted = false;
										continue;
									}
									else { // give event room C
										printf("child %i, give room C\n",i);
										assigned_room = 'C';
										isAccepted = true;
														
										// time slot for testing
										// printTimeSlot(t_slot,i);
									}
								}
								else { // give event room B
									printf("child %i, give room B\n",i);
									assigned_room = 'B';
									isAccepted = true;
										
									// time slot for testing
									// printTimeSlot(t_slot,i);
								}
							}
							else { // give event room A
								printf("child %i, give room A\n",i);
								assigned_room = 'A';
								isAccepted = true;
								
								// time slot for testing
								// printTimeSlot(t_slot,i);
							}
						}
						// >10 person, must use room C
						else if (room_C == false) {
							isAccepted = false;
							continue;	// room C not available
						}
						else { // give event room C
							printf("child %i, give room C\n",i);
							assigned_room = 'C';
							isAccepted = true;

							// time slot for testing
							// printTimeSlot(t_slot,i);
						}
					}
					// else 0 people is in the bookDevice event, skip

					if ((strcmp(eventList[j].deviceA, str_webcam_FHD) == 0) || (strcmp(eventList[j].deviceB, str_webcam_FHD) == 0))
					{
						printf("child %i, webcam_FHD\n",i);
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on first webcam
							if (t_slot.webcam_FHD_1[time + d] >= 0) {
								webcam_FHD_1 = false;
							}
							if (t_slot.webcam_FHD_2[time + d] >= 0) {
								webcam_FHD_2 = false;
							}
						}
						if (webcam_FHD_1 == false) {	
							if (webcam_FHD_2 == false) {	// both webcam is busy, skip
								isAccepted = false;
								continue;
							}
							else {
								isAccepted = true;// webcam 2 is ok
							}
						}
						else {
							webcam_FHD_2 = false;	// webcam 1 is ok, not use webcam 2
							isAccepted = true;	
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need webcam FHD
						webcam_FHD_1 = false;
						webcam_FHD_2 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_webcam_UHD) == 0) || (strcmp(eventList[j].deviceB, str_webcam_UHD) == 0))
					{
						printf("child %i, webcam_UHD\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.webcam_UHD[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								webcam_UHD = false;
							}
						}
						if (webcam_UHD == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {					
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need webcam uHD
						webcam_UHD = false;
					}

					if ((strcmp(eventList[j].deviceA, str_monitor_50) == 0) || (strcmp(eventList[j].deviceB, str_monitor_50) == 0))
					{		
						printf("child %i, monitor_50\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on the 2 monitors
							if (t_slot.monitor_50_1[time + d] >= 0) {
								monitor_50_1 = false;
							}
							if (t_slot.monitor_50_2[time + d] >= 0) {
								monitor_50_2 = false;
							}
						}
						if (monitor_50_1 == false) {
							if (monitor_50_2 == false) {	// check if second monitor is also occupied
								isAccepted = false;
								continue;			// both monitor is busy, skip
							}
							else {
								isAccepted = true;
							}
						}
						else {
							monitor_50_2 = false;	// monitor 1 is available, not use monitor 2
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need monitor 50
						monitor_50_1 = false;
						monitor_50_2 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_monitor_75) == 0) || (strcmp(eventList[j].deviceB, str_monitor_75) == 0))
					{
						printf("child %i, monitor_75\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.monitor_75[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								monitor_75 = false;
							}
						}
						if (monitor_75 == false) {
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							for(d = 0; d < duration; d++) {
								t_slot.monitor_75[time + d] = j;
							}
							isAccepted = true;
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need monitor 75
						monitor_75 = false;
					}

					if ((strcmp(eventList[j].deviceA, str_projector_2K) == 0) || (strcmp(eventList[j].deviceB, str_projector_2K) == 0))
					{		
						printf("child %i, projector_2K\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;								
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on projector_2k 1
							if (t_slot.projector_2k_1[time + d] >= 0) {
								projector_2k_1 = false;
							}
							if (t_slot.projector_2k_2[time + d] >= 0) {
								projector_2k_2 = false;
							}
						}

						if (projector_2k_1 == false) {	
							if (projector_2k_2 == false) {	// check if projector_2k 2 is also occupied
								isAccepted = false;
								continue; // both projector_2k is busy, skip
							}
							else {
								isAccepted = true;	// projector_2k 2 is available
							}
						}
						else {
							projector_2k_2 = false;
							isAccepted = true;	// projector_2k 1 is available, not use screen_100 2
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					else {
						// no need projector 2k
						projector_2k_1 = false;
						projector_2k_2 = false;
					}
					

					if ((strcmp(eventList[j].deviceA, str_projector_4K) == 0) || (strcmp(eventList[j].deviceB, str_projector_4K) == 0))
					{		
						printf("child %i, projector_4K\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;
						bool isOccupied = false;								
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.projector_4k[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								projector_4k = false;
							}
						}
						if (projector_4k == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							isAccepted = true;
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);				
					}
					else {
						// no need projector 4k
						projector_4k = false;
					}

					// printf("strcmp for event %i is %i or %i\n", eventList[j].event_num, strcmp(eventList[j].deviceA, str_screen_100),strcmp(eventList[j].deviceB, str_screen_100));

					if ((strcmp(eventList[j].deviceA, str_screen_100) == 0) || (strcmp(eventList[j].deviceB, str_screen_100) == 0))
					{			
						printf("child %i, screen_100\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) {	// loop from time start to time + duration on screen_100 1/2
							if (t_slot.screen_100_1[time + d] >= 0) {
								screen_100_1 = false;
							}
							if (t_slot.screen_100_2[time + d] >= 0) {
								screen_100_2 = false;
							}
						}

						if (screen_100_1 == false) {	
							if (screen_100_2 == false) {	// check if screen_100 2 is also occupied
								isAccepted = false;
								continue; // both screen_100 is busy, skip
							}
							else {
								isAccepted = true;	// screen_100 2 is available
							}
						}
						else {
							screen_100_2 = false;
							isAccepted = true;	// screen_100 1 is available, not use screen_100 2
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);				
					}
					else {
						// no need screen 100
						screen_100_1 = false;
						screen_100_2 = false;
					}
					
					if ((strcmp(eventList[j].deviceA, str_screen_150) == 0) || (strcmp(eventList[j].deviceB, str_screen_150) == 0))
					{		
						printf("child %i, screen_150\n",i);
						// check if all the time slot needed is blank
						// d is a offset of the time
						int d;							
						for(d = 0; d < duration; d++) { // loop from time start to time + duration
							if (t_slot.screen_150[time + d] >= 0) {	// if one of the time slot is occupied, it cannot be used
								screen_150 = false;
							}
						}
						if (screen_150 == false) {
							isAccepted = false;
							continue;	// time slot is occupied, continue
						}
						else {	// fill time slot with event number
							isAccepted = true;
						}	
						// time slot for testing
						// printTimeSlot(t_slot,i);						
					}
					else {
						// no need screen 150
						screen_150 = false;
					}

					// all the requested room and device are available, proceed to add to accpeted event list and write on the time slot
					if (isAccepted == true) {
						printf("accepted event %i\n", j);
						// copy the event to the accepted event list, then modify the accepted state and assigned room
						accepted_eventList[accepted_events] = eventList[j];	// copy the current event to a new list
						accepted_eventList[accepted_events].accepted_state = true;
						if (assigned_room) {
							accepted_eventList[accepted_events].room = assigned_room;	// assign room to the event									
						}
						accepted_events++;

						// fill the time slot for later checking
						int d;

						// fill room time slot, only one room will be filled
						switch (assigned_room)
						{
						case 'A': {
							for (d = 0; d < duration; d++) {
								t_slot.roomA[time + d] = j;
							}
							break;
						}
						case 'B': {
							for (d = 0; d < duration; d++) {
								t_slot.roomB[time + d] = j;
							}
							break;
						}
						case 'C': {
							for (d = 0; d < duration; d++) {
								t_slot.roomC[time + d] = j;
							}
							break;
						}								
						default:
							break;
						}

						// fill webcam_FHD time slot, only one of the webcam will be filled
						if (webcam_FHD_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.webcam_FHD_1[time + d] = j;
							}
						}
						if (webcam_FHD_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.webcam_FHD_2[time + d] = j;
							}
						}

						// fill webcam_UHD time slot
						if (webcam_UHD == true) {
							for(d = 0; d < duration; d++) {
								t_slot.webcam_UHD[time + d] = j;
							}
						}

						// fill monitor_50 time slot, only one of the monitors will be filled
						if (monitor_50_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_50_1[time + d] = j;
							}
						}
						if (monitor_50_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_50_2[time + d] = j;
							}
						}

						// fill monitor_75 time slot
						if (monitor_75 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.monitor_75[time + d] = j;
							}
						}

						// fill projector_2K time slot, only one of the projectors will be filled
						if (projector_2k_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_2k_1[time + d] = j;
							}
						}
						if (projector_2k_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_2k_2[time + d] = j;
							}
						}

						// fill projector_4K time slot
						if (projector_4k == true) {
							for (d = 0; d < duration; d++) {
								t_slot.projector_4k[time + d] = j;
							}
						}

						// fill screen_100 time slot, only one of the screens will be filled
						if (screen_100_1 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_100_1[time + d] = j;
							}
						}
						if (screen_100_2 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_100_2[time + d] = j;
							}
						}

						// fill screen_150 time slot
						if (screen_150 == true) {
							for (d = 0; d < duration; d++) {
								t_slot.screen_150[time + d] = j;
							}
						}
						// time slot for testing
						// printTimeSlot(t_slot,i);
					}
					// if not accepted, it will be skipped
				}
				
			 	}
			 	else
			 		continue;
			}

			printf("child %i, finished checking\n\n",i);
			close(fd[i][0]);
			if (accepted_events > 0) {	// if there is accepted events					
				printf("sending %i events\n", accepted_events);
				write(fd[i][1], &accepted_eventList, sizeof(*eventList)*MAX_EVENT);	// tell parent the accepted event number list
			}
			else {	// if there is no accepted events
				//initialie event data	(date, time, duration, persons, initialize state is false)				
				printf("sending blank event\n\n");				
				struct Event blank = {
					.duration = -1.0,
				};
				write(fd[i][1], &blank, sizeof(*eventList)*MAX_EVENT);	// no event is accepted that day, return a blank event to parent
			}
			close(fd[i][1]);

			printf("child %i, start combining time slot count\n",i);
			// ultilization of slots of all devices/room
			int h;
			for (h = 0; h < 24; h++) {				
				if (t_slot.roomA[h] >= 0) { timeSlot_ultilization[0]++; }
				if (t_slot.roomB[h] >= 0) { timeSlot_ultilization[1]++; }
				if (t_slot.roomC[h] >= 0) { timeSlot_ultilization[2]++; }
				if (t_slot.webcam_FHD_1[h] >= 0) { timeSlot_ultilization[3]++; }
				if (t_slot.webcam_FHD_2[h] >= 0) { timeSlot_ultilization[4]++; }
				if (t_slot.webcam_UHD[h] >= 0) { timeSlot_ultilization[5]++; }
				if (t_slot.monitor_50_1[h] >= 0) { timeSlot_ultilization[6]++; }
				if (t_slot.monitor_50_2[h] >= 0) { timeSlot_ultilization[7]++; }
				if (t_slot.monitor_75[h] >= 0) { timeSlot_ultilization[8]++; }
				if (t_slot.projector_2k_1[h] >= 0) { timeSlot_ultilization[9]++; }
				if (t_slot.projector_2k_2[h] >= 0) { timeSlot_ultilization[10]++; }
				if (t_slot.projector_4k[h] >= 0) { timeSlot_ultilization[11]++; }
				if (t_slot.screen_100_1[h] >= 0) { timeSlot_ultilization[12]++; }
				if (t_slot.screen_100_2[h] >= 0) { timeSlot_ultilization[13]++; }
				if (t_slot.screen_150[h] >= 0) { timeSlot_ultilization[14]++; }
			}
			// total accepted bookings
			timeSlot_ultilization[15] += accepted_events;

			// sending ultilization of time slots to parent
			printf("child %i, sending ultilization of time slots\n\n",i);

			close(fd_timeSlot[i][0]);
			write(fd_timeSlot[i][1], timeSlot_ultilization, sizeof(*timeSlot_ultilization)*16);	// tell parent the accepted event number list
			close(fd_timeSlot[i][1]);

			exit(0);
		}
		// sleep(1);
	}
	

	int *full_timeSlot_ultilization = malloc(sizeof(int)*16);
	int ts;
	for (ts = 0; ts < 16; ts++) {
		*(full_timeSlot_ultilization+ts) = 0;
	}

	// collect child
	printf("collecting pipe\n");
	for (i = 0; i < 7; i++) {				
		wait(NULL);
		close(fd[i][1]);
		struct Event accepted_arr[MAX_EVENT];
		int m;
		int timeSlot_ultilization[16];
		// read the array that child sent if there is accepted events
		while(1) {
			// sleep(1);
			m = read(fd[i][0], accepted_arr, sizeof(*eventList)*MAX_EVENT);
			// printf("<<<<<<<<<   m is %i, first duration is %.1f\n",m,accepted_arr[0].duration);
			if (m >= 0) {
				printf("child %i returned array, size %i\n",i,m);
				break;
			}
		}

		printf("\nsee what child %i send back\n",i);
		// for(m = 0; m < MAX_EVENT; m++) {
		// 	if (accepted_arr[m].duration > 0.0)
		// 		printEvent(accepted_arr[m],m);
		// }

		// for test print
		printf("child %i, start inserting back to returned array\n",i);
		int c;
		for (c = 0; c < MAX_EVENT; c++) {
			// early teminating, duration cannot be 0, skip to next date
			// printf("the event is %s\n",&accepted_arr[c].event);
			if (accepted_arr[c].duration < 1.0 || accepted_arr[c].accepted_state == false || accepted_arr[c].validaty == false) {
				printf("date 1%i/5, early terminate, no more output\n",i);
				break;
			}
			// print the accepted event					
			printf("replacing event %i with info from event %i: %s\n",accepted_arr[c].event_num,c,accepted_arr[c].event);
			int replacing_event_num = accepted_arr[c].event_num;



			// writing room
			eventList[replacing_event_num].room = accepted_arr[c].room;
			// printf("[[[[[[[[[ accepted is %c, writing %c to event %i\n",accepted_arr[c].room,eventList[replacing_event_num].room,replacing_event_num);
			
			eventList[replacing_event_num].accepted_state = accepted_arr[c].accepted_state;
			
			printEvent(eventList[replacing_event_num],replacing_event_num);
		}
				
		printf("child %i, getting time slot array\n",i);
		// read the time slot ultilization array that child sent
		while(1) {
			// sleep(1);
			m = read(fd_timeSlot[i][0], timeSlot_ultilization, sizeof(*timeSlot_ultilization)*16);
			// printf("<<<<<<<<<   m is %i, first duration is %.1f\n",m,accepted_arr[0].duration);
			if (m >= 0) {
				printf("child %i returned time slot array, size %i\n",i,m);
				break;
			}
		}
		
		printf("child %i, start inserting back to returned array\n",i);
		for (c = 0; c < 16; c++) {
			full_timeSlot_ultilization[c] += timeSlot_ultilization[c];
		}

		printf("\n");
		close(fd[i][0]);
		close(fd_timeSlot[i][0]);
		// free(accepted_arr);
		// sleep(1);
	}

	// print the result based on the algorithm				
	printByTenants(eventList,"prio",total_event);

	return full_timeSlot_ultilization;
}


int main() {

    // initializing input line and output file
    char inputCommand[120];
    FILE *outputFile;

    int i = 0;	// input command pointer
	int j = 0;	// for test only
	int k = 0;	// event list counter
    struct tm *time;	// store the date and time of an event
    struct Event eventList[MAX_EVENT];	// event list to store all requests
	struct Event temp[1];	// empty event list

	int *timeSlot_FCFS;		// store the time slot ultilization data of FCFS
	int *timeSlot_PRIO;		// store the time slot ultilization data of PRIO

    int invalidCommand = 0;

    /* open a txt file for the outputs
	
    outputFile = fopen("RBM_Report_G16.txt","w");
    if (outputFile == NULL) {
        printf("File creation error!");
    }
	*/

    // start up message
    printf("~~ WELCOME TO PolySME ~~\n");
    
    // RBM Kernal
    while(1) {
    	i = 0, j = 0;
        char *event_get;
    	
        // Get command input from user
        printf("Please enter booking: \n");
        scanf("%[^\n]%*c", inputCommand);
        if(inputCommand[strlen(inputCommand)-1] == ';')
			inputCommand[strlen(inputCommand)-1] = '\0';	//erase ";"
        printf("You entered %s\n",inputCommand);

        // use space to seperate input line
        char *inputCommand_ptr[9];
        inputCommand_ptr[i] = strtok(inputCommand, " ");

        // store every parameter in array
        while (inputCommand_ptr[i] != NULL) {
            inputCommand_ptr[++i] = strtok(NULL," ");
        }
        printf("Stored inputs.\n");

		// the command is add*, but not addBatch
		if (strcmp(inputCommand_ptr[0],"addMeeting") == 0 || 
		strcmp(inputCommand_ptr[0],"addPresentation") == 0 || 
		strcmp(inputCommand_ptr[0],"addConference") == 0) {
			struct Event temp = createEvent(inputCommand_ptr,i);
			if (temp.validaty == true) {
				eventList[k] = temp;
				eventList[k].event_num = k;

				//print screen for test
				printEvent(eventList[k],k);
				k++;
			}
			else {
				invalidCommand++;
			}
		}		
		
		// book device
        else if (strcmp(inputCommand_ptr[0],"bookDevice") == 0) {
			struct Event temp = createEvent(inputCommand_ptr,i);
			if (temp.validaty == true) {
				eventList[k] = temp;
				eventList[k].event_num = k;

				//print screen for test
				printEvent(eventList[k],k);
				k++;
			}
			else {
				invalidCommand++;
			}
		}
		
		// add batch of commands
        else if (strcmp(inputCommand_ptr[0],"addBatch") == 0) {
            printf("Command is addBatch.\n");
			// get the batch file name
			char *batchName = inputCommand_ptr[1];
			batchName++;
			//sprintf(batchName, "%s", batchName+1);
			printf("bat: %s\n",batchName);

			// open current directory
			DIR *dirp;
			struct dirent *dir;

			dirp = opendir(".");
			if (dirp == NULL) {
				printf("opendir error\n");
				invalidCommand++;
				continue;
			}

			// open the batch file
			FILE *fp;
            char *line = NULL;
            size_t len = 0;
            ssize_t read;

			fp = fopen(batchName, "r");
			if (fp == NULL) {
				printf("Failed to open file.\n");
				invalidCommand++;
				continue;
			}

			// read every line of the batch file
			char *batchCommand;
			while ((read = getline(&line, &len, fp)) != -1) {				
				i = 0;
				batchCommand = line;
				
				// use space to seperate input line
				char *inputCommand_ptr[9];
				inputCommand_ptr[i] = strtok(batchCommand, " ");

				// store every parameter in array
				while (inputCommand_ptr[i] != NULL) {
					inputCommand_ptr[++i] = strtok(NULL," ");
					// printf(">>>>>Command %i, %s\n",k,inputCommand_ptr[i-1]);
				}
				
				
				printf("len: %ld\n",strlen(inputCommand_ptr[i-1]));
				
				for(j=0;j<strlen(inputCommand_ptr[i-1]);j++){
					if(inputCommand_ptr[i-1][j] == ';'){
						inputCommand_ptr[i-1][j] = '\0';
					}
				}
				
				
				printf("Stored inputs, k = %i.\n",k);

				struct Event temp = createEvent(inputCommand_ptr,i);
				if (temp.validaty == true) {
					eventList[k] = temp;
					eventList[k].event_num = k;

					//print screen for test
					printEvent(eventList[k],k);
					k++;
				}
				else {
					invalidCommand++;
				}
				
			}
			fclose(fp);
			closedir(dirp);
        }

		// print all bookings
        else if (strcmp(inputCommand_ptr[0],"printBookings") == 0) {
			// print the result based on the algorithm
			if(strcmp(inputCommand_ptr[1],"-fcfs") == 0) {
				// reset eventlist states
				for(j=0; j < k; j++){
					if(eventList[j].validaty == true) {
						eventList[j].accepted_state = false;
						eventList[j].room = ' ';
					}
				}	
				timeSlot_FCFS = printFCFS(eventList,k);
			}

			else if (strcmp(inputCommand_ptr[1],"-prio") == 0) {
				// reset eventlist states
				for(j=0; j < k; j++){
					if(eventList[j].validaty == true) {
						eventList[j].accepted_state = false;
						eventList[j].room = ' ';
					}
				}	
				timeSlot_PRIO = printPRIO(eventList,k);

			}

			else if(strcmp(inputCommand_ptr[1],"-ALL") == 0) {
				// reset eventlist states
				for(j=0; j < k; j++){
					if(eventList[j].validaty == true) {
						eventList[j].accepted_state = false;
						eventList[j].room = ' ';
					}
				}
				timeSlot_FCFS = printFCFS(eventList,k);

				printf("fcfs finished\n");

				// reset eventlist states
				for(j=0; j < k; j++){
					if(eventList[j].validaty == true) {
						printf("resetting for prio: event %i is %s\n",j,eventList[j].event);
						eventList[j].accepted_state = false;
						eventList[j].room = ' ';
					}
				}
				timeSlot_PRIO = printPRIO(eventList,k);

				printf("prio finished\n");
				// loop to false and room=' '
				// timeSlot_OPTI = printOPTI(eventList,k);
				printSummary(timeSlot_FCFS,timeSlot_PRIO,k,invalidCommand);

				printf("summary finished\n");
				// printSummary(timeSlot_FCFS,timeSlot_PRIO,timeSlot_OPTI,k);
			}
			else{
				printf("Invalid command. Please enter: printBookings -xxx -[fcfs/prio/ALL]\n");
				invalidCommand++;
			}
			
		}

        else if (strcmp(inputCommand_ptr[0],"endProgram") == 0) {
            printf("Bye!\n");
            exit(0);
        }

        else {
            printf("Invalid command. Please use:\n - addMeeting\n - addPresentation\n - addConference\n - bookDevice\n - addBatch\n - printBookings\n - endProgram\n");
			invalidCommand++;
        }
    }
    
    exit(0);
}
