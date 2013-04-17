#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct thread{
	int prio;
	int inv_weight;
	int vr;
};

static int weight_list[64] = 
{1, 2, 3, 4, 6, 8, 9, 12, 16, 18, 
24, 27, 32, 36, 48, 54, 64, 72, 81, 96, 
108, 128, 144, 162, 192, 216, 243, 288, 324, 384, 
432, 486, 576, 648, 729, 864, 972, 1152, 1296, 1458, 
1728, 1944, 2187, 2592, 2916, 3456, 3888, 4374, 5184, 5832, 
7776, 8748, 10368, 11664, 15552, 17496, 23328, 31104, 34992, 46656, 
69984, 93312, 139968, 279936};


static int inv_weight_list[64] =
{279936, 139968, 93312, 69984, 46656, 34992, 31104, 23328, 17496, 15552,
11664, 10368, 8748, 7776, 5832, 5184, 4374, 3888, 3456, 2916,
2592, 2187, 1944, 1728, 1458, 1296, 1152, 972, 864, 729,
648, 576, 486, 432, 384, 324, 288, 243, 216, 192,
162, 144, 128, 108, 96, 81, 72, 64, 54, 48,
36, 32, 27, 24, 18, 16, 12, 9, 8, 6,
4, 3, 2, 1};

int find_min_vr(struct thread *t, int n){
	int min_vr = t[0].vr;
	int min_prio = t[0].prio;
	int i, min = 0;
	for(i = 1; i < n; i ++){
		if(t[i].vr < min_vr){
			min_vr = t[i].vr;
			min_prio = t[i].prio;
			min = i;
		}
		else if(t[i].vr == min_vr){
			if(t[i].prio > min_prio){
				min_vr = t[i].vr;
				min_prio = t[i].prio;
				min = i;
			}
		}
	}

	return min;
}

int main()
{
	/*
	struct thread t[6];
	int i, to_change;

	for(i = 0; i < 6; i ++){
		t[i].prio = i;
		t[i].inv_weight = inv_weight_list[i];
		t[i].vr = t[i].inv_weight;
	}

	for(i = 0; i < 100; i ++){
			to_change = find_min_vr(t);
			t[to_change].vr += t[to_change].inv_weight;
			printf("thread number: %d\n", t[to_change].prio);
	}
	*/

	struct thread *my_thread, *ideal_thread;
	int *prio_input;
	int my_step, ticks;
	int **my_vr, **ideal_vr, **service_time, **gps;

	int n, i, j, to_change, steps;
	FILE *fr, *fw;
	char p1[256], p2[256];

	printf("만든 쓰레드 개수? :");
	scanf("%d", &n);

	printf("총 스텝 수? :");
	scanf("%d", &steps);

	prio_input = (int*)malloc(sizeof(int)*n);
	printf("만든 프라이오리티 순서? :");

	for(i = 0; i < n; i ++){
		scanf("%d", &prio_input[i]);
	}

	

	my_thread = (struct thread*)malloc(sizeof(struct thread)*n);
	ideal_thread = (struct thread*)malloc(sizeof(struct thread)*n);

	my_vr = (int**)malloc(sizeof(int)*(steps+1));
	ideal_vr = (int**)malloc(sizeof(int)*(steps+1));
	service_time = (int**)malloc(sizeof(int)*(steps+1));
	gps = (int**)malloc(sizeof(int)*(steps+1));

	for(i = 0; i <= steps; i ++){
		my_vr[i] = (int*)malloc(sizeof(int)*n);
		ideal_vr[i] = (int*)malloc(sizeof(int)*n);
		service_time[i] = (int*)malloc(sizeof(int)*n);
		gps[i] = (int*)malloc(sizeof(int)*n);
	}

	//initialize
	for(i = 0; i < n; i++){
		my_thread[i].prio = ideal_thread[i].prio = prio_input[i];
		my_thread[i].inv_weight = ideal_thread[i].inv_weight = inv_weight_list[prio_input[i]];
		my_thread[i].vr = ideal_thread[i].vr = inv_weight_list[prio_input[i]];
		for(j = 0; j <= steps; j++){
			my_vr[j][i] = ideal_vr[j][i] = inv_weight_list[prio_input[i]];
			service_time[j][i] = gps[j][i] = 0;
		}
	}

	//make gps_base
	ticks = 0;
	for(i = 0; i < n; i++){
		ticks += weight_list[prio_input[i]];
	}

	
	
	//make ideal_thread
	for(i = 0; i < steps; i++){
		to_change = find_min_vr(ideal_thread, n);
		ideal_thread[to_change].vr += ideal_thread[to_change].inv_weight;
		ideal_vr[i+1][to_change] = ideal_vr[i][to_change] + ideal_thread[to_change].inv_weight;
		
		for(j = 0; j < n; j ++){
			//make gps
			gps[i+1][j] = gps[i][j] + weight_list[prio_input[j]];
			if(j == to_change)
				continue;
			else{
				ideal_vr[i+1][j] = ideal_vr[i][j];
			}
		}
	}

	//make my_thread
	if((fr=fopen("input4.txt", "r")) == NULL){
		fprintf(stderr, "Error");
		exit(1);
	}
	if((fw=fopen("output4.txt", "w")) == NULL){
		fprintf(stderr, "Error");
		exit(1);
	}


	for(i = 0; i < steps; i++){
		fscanf(fr, "%d %d", &to_change, &my_step);

		for(j = 0; j < n; j++){
			if(prio_input[j] == my_step)
				break;
		}
		my_step = j;

		my_vr[i+1][my_step] = my_vr[i][my_step] + my_thread[my_step].inv_weight;
		
		//make service time
		service_time[i+1][my_step] = service_time[i][my_step] + ticks;
		
		for(j = 0; j < n; j++){
			if(j == my_step)
				continue;
			else
				my_vr[i+1][j] = my_vr[i][j];
				service_time[i+1][j] = service_time[i][j];
		}
	}

	for(i = 0; i <= steps; i ++){
		fprintf(fw, "my: ");
		for(j = 0; j < n; j++)
			fprintf(fw, "%d ", my_vr[i][j]);
		
		fprintf(fw, " ideal: ");
		for(j = 0; j < n; j++)
			fprintf(fw, "%d ", ideal_vr[i][j]);
		
		fprintf(fw, " service time: ");
		for(j = 0; j < n; j++)
			fprintf(fw, "%d ", service_time[i][j]);
			
		fprintf(fw, " gps: ");
		for(j = 0; j < n; j++)
			fprintf(fw, "%d ", gps[i][j]);

		fprintf(fw, " s_t_e: ");
		for(j = 0; j < n; j++)
			fprintf(fw, "%d ", service_time[i][j]-gps[i][j]);

		fprintf(fw, "\n");
	}
	
	//terminate
	fclose(fr);
	fclose(fw);

	free(my_thread);
	free(ideal_thread);

	for(i = 0; i <= steps; i++){
		free(my_vr[i]);
		free(ideal_vr[i]);
		free(service_time[i]);
		free(gps[i]);
	}

	free(my_vr);
	free(ideal_vr);
	free(service_time);
	free(gps);




	return 0;
}
