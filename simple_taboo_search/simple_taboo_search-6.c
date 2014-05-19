#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "fifo.h"	/* for taboo list */


#define MAXSIZE (512)

#define TABOOSIZE (1000)
#define BIGCOUNT (9999999)

#define min(a,b) (((a) < (b)) ? (a) : (b))



/***
 *** example of very simple search for R(6,6) counter examples
 ***
 *** starts with a small randomized graph and works its way up to successively
 *** larger graphs one at a time
 ***
 *** uses a taboo list of size #TABOOSIZE# to hold and encoding of and edge
 *** (i,j)+clique_count
 ***/

/*
 * PrintGraph
 *
 * prints in the right format for the read routine
 */
void PrintGraph(int *g, int gsize)
{
	int i;
	int j;

	fprintf(stdout,"%d\n",gsize);

	for(i=0; i < gsize; i++)
	{
		for(j=0; j < gsize; j++)
		{
			fprintf(stdout,"%d ",g[i*gsize+j]);
		}
		fprintf(stdout,"\n");
	}

	return;
}

/*
 * CopyGraph 
 *
 * copys the contents of old_g to corresponding locations in new_g
 * leaving other locations in new_g alone
 * that is
 * 	new_g[i,j] = old_g[i,j]
 */
void CopyGraph(int *old_g, int o_gsize, int *new_g, int n_gsize)
{
	int i;
	int j;

	/*
	 * new g must be bigger
	 */
	if(n_gsize < o_gsize)
		return;

	for(i=0; i < o_gsize; i++)
	{
		for(j=0; j < o_gsize; j++)
		{
			new_g[i*n_gsize+j] = old_g[i*o_gsize+j];
		}
	}

	return;
}


/*
 ***
 *** returns the number of monochromatic cliques in the graph presented to
 *** it
 ***
 *** graph is stored in row-major order
 *** only checks values above diagonal
 */

int CliqueCount(int *g,
	     int gsize)
{
    int i;
    int j;
    int k;
    int l;
    int m;
    int n;
    int count=0;
    int sgsize = 6;
    
    for(i=0;i < gsize-sgsize+1; i++)
    {
	for(j=i+1;j < gsize-sgsize+2; j++)
        {
	    for(k=j+1;k < gsize-sgsize+3; k++) 
            { 
		if((g[i*gsize+j] == g[i*gsize+k]) && 
		   (g[i*gsize+j] == g[j*gsize+k]))
		{
		    for(l=k+1;l < gsize-sgsize+4; l++) 
		    { 
			if((g[i*gsize+j] == g[i*gsize+l]) && 
			   (g[i*gsize+j] == g[j*gsize+l]) && 
			   (g[i*gsize+j] == g[k*gsize+l]))
			{
			    for(m=l+1;m < gsize-sgsize+5; m++) 
			    {
				if((g[i*gsize+j] == g[i*gsize+m]) && 
				   (g[i*gsize+j] == g[j*gsize+m]) &&
				   (g[i*gsize+j] == g[k*gsize+m]) && 
				   (g[i*gsize+j] == g[l*gsize+m])) {
					for(n=m+1; n < gsize-sgsize+6; n++)
					{
						if((g[i*gsize+j]
							== g[i*gsize+n]) &&
						   (g[i*gsize+j] 
							== g[j*gsize+n]) &&
						   (g[i*gsize+j] 
							== g[k*gsize+n]) &&
						   (g[i*gsize+j] 
							== g[l*gsize+n]) &&
						   (g[i*gsize+j] 
							== g[m*gsize+n])) {
			      					count++;
						}
					}
				}
			    }
			}
		    }
		}
	    }
         }
     }
    return(count);
}



int simulatedAnnealing(int *gPointer, int *gsizePointer, int *iPointer, int *jPointer, int *best_iPointer, 
	int *best_jPointer, int *best_countPointer, void *taboo_listPointer, 
	double *temprature_maxPointer, double *dtPointer, double *temperaturePointer)
{
	int *g = gPointer;
	int gsize = *gsizePointer;

	int count;
	
	int i = *iPointer;
	int j = *jPointer;
	int best_i = *best_iPointer;
	int best_j = *best_jPointer;
	int best_count = *best_countPointer;
	void *taboo_list = 	taboo_listPointer;
	double temprature_max = *temprature_maxPointer;
	double dt = *dtPointer;
	double temperature = *temperaturePointer;


	double evFunction;
	double randomNumber;


	printf("hello simulated annealing\n");


	//lets do simulated annealing instead

	for (int i = 0; i < gsize; ++i)
	{
		for (int j = i+1; j < gsize; ++j)
		{
			//return if we have found a solution
			if (best_count == 0)
			{
				*gPointer = *g;
				*gsizePointer = gsize;
				*iPointer = i;
				*jPointer = j;
				*best_iPointer = best_i;
				*best_jPointer = best_j;
				*best_countPointer = best_count;
				taboo_listPointer = taboo_list;
				*temprature_maxPointer = temprature_max;
				*dtPointer = dt;
				*temperaturePointer = temperature;

				return(0);
			}

			//flip and check count
			g[i*gsize+j] = 1-g[i*gsize+j];

			count = CliqueCount(g, gsize);

			//evaluate the solution

			if(count < best_count)
			{
				evFunction = 1.0;
			}
			else
			{
				evFunction = exp( (best_count - count) / temperature );
			}

			//flip it back
			g[i*gsize+j] = 1-g[i*gsize+j];

			randomNumber = (double)(rand() % 10) / 10.0;

			// printf("random number is %f\n", randomNumber);
			// printf("ev function is %f\n", evFunction);

			if (evFunction > 0.999997 || evFunction==1.0)
			{
				// printf("found better solution\n");
				best_i = i;
				best_j = j;
				best_count = count;
			}
			// else
			// 	{
			// 		printf("let's explore\n");
			// 	}

			//decrease the temprature
			if(temperature-dt <= 0)
			{
				temperature = 0;
				printf("error: stopped before solution was found\n");
				*gPointer = *g;
				*gsizePointer = gsize;
				*iPointer = i;
				*jPointer = j;
				*best_iPointer = best_i;
				*best_jPointer = best_j;
				*best_countPointer = best_count;
				taboo_listPointer = taboo_list;
				*temprature_maxPointer = temprature_max;
				*dtPointer = dt;
				*temperaturePointer = temperature;

				return(0);
			}
			else
			{
				temperature = temperature-dt;
			}
			

		}

		// if (i!=best_i)
		// {
		// 	printf("have to select a random neighbor\n");
		// 	int rand1 = rand() % gsize;
		// 	int rand2 = rand() % gsize;

		// 	g[rand1*gsize+rand2] = 1-g[rand1*gsize+rand2];
		// 	count = CliqueCount(g, gsize);

		// 	best_i = rand1;
		// 	best_j = rand2;
		// 	best_count = count;

		// }

		if (i==best_i)
		{
			g[best_i*gsize+best_j] = 1 - g[best_i*gsize+best_j];

			printf("ce size: %d, best_count: %d, best edge: (%d,%d), new color: %d\n",
			gsize,
			best_count,
			best_i,
			best_j,
			g[best_i*gsize+best_j]);
		}
		

		
	}

	*gPointer = *g;
	*gsizePointer = gsize;
	*iPointer = i;
	*jPointer = j;
	*best_iPointer = best_i;
	*best_jPointer = best_j;
	*best_countPointer = best_count;
	taboo_listPointer = taboo_list;
	*temprature_maxPointer = temprature_max;
	*dtPointer = dt;
	*temperaturePointer = temperature;

	return(0);
}



void tabooSearch(int *gPointer, int *new_gPointer, int *gsizePointer, int *countPointer, int *iPointer, int *jPointer,
	int *best_countPointer, int *best_iPointer, int *best_jPointer, void *taboo_listPointer, 
	double *temprature_maxPointer, double *dtPointer, double *temperaturePointer)
{
	
	int *g = gPointer;
	int *new_g = new_gPointer;
	int gsize = *gsizePointer;
	int count = *countPointer;
	int i = *iPointer;
	int j = *jPointer;
	int best_i = *best_iPointer;
	int best_j = *best_jPointer;
	int best_count = *best_countPointer;
	void *taboo_list = taboo_listPointer;
	double temprature_max = *temprature_maxPointer;
	double dt = *dtPointer;
	double temperature = *temperaturePointer;


	/*
	 * while we do not have a publishable result
	 */
	 while(gsize < 102)
	 {
		/*
		 * find out how we are doing
		 */
		count = CliqueCount(g,gsize);

		/*
		 * if we have a counter example
		 */
		if(count == 0)
		{
			printf("Eureka!  Counter-example found!\n");
			PrintGraph(g,gsize);
			/*
			 * make a new graph one size bigger
			 */
			 new_g = (int *)malloc((gsize+1)*(gsize+1)*sizeof(int));
			 if(new_g == NULL)
			 	exit(1);
			/*
			 * copy the old graph into the new graph leaving the
			 * last row and last column alone
			 */
			 CopyGraph(g,gsize,new_g,gsize+1);

			/*
			 * zero out the last column and last row
			 */

			 new_g[i*(gsize+1) + gsize] = 0; // last column
			 new_g[gsize*(gsize+1) + i] = 0; // last row

			// for(i=0; i < (gsize+1); i++)
			// {
			// 	int r = rand() % 1;
			// 	if (r < 0.5)
			// 	{
			// 		new_g[i*(gsize+1) + gsize] = 0; // last column
			// 		new_g[gsize*(gsize+1) + i] = 1; // last row
			// 	}
			// 	else
			// 	{
			// 		new_g[i*(gsize+1) + gsize] = 1; // last column
			// 		new_g[gsize*(gsize+1) + i] = 0; // last row
			// 	}
				
			// }

			/*
			 * throw away the old graph and make new one the
			 * graph
			 */
			 free(g);
			 g = new_g;
			 gsize = gsize+1;

			/*
			 * reset the taboo list for the new graph
			 */
			taboo_list = FIFOResetEdge(taboo_list);

			/*
			 * keep going
			 */
			continue;
			//break;
		}

		/*
		 * otherwise, we need to consider flipping an edge
		 *
		 * let's speculative flip each edge, record the new count,
		 * and unflip the edge.  We'll then remember the best flip and
		 * keep it next time around
		 *
		 * only need to work with upper triangle of matrix =>
		 * notice the indices
		 */

		best_count = BIGCOUNT;
		for(i=0; i < gsize; i++)
		{

			for(j=i+1; j < gsize; j++)
			{

				if (best_count < 5500)
				{
					*gPointer = *g;
					*new_gPointer = *new_g;
					*gsizePointer = gsize;
					*countPointer = count;
					*iPointer = i;
					*jPointer = j;
					*best_iPointer = best_i;
					*best_jPointer = best_j;
					*best_countPointer = best_count;
					taboo_listPointer = taboo_list;
					*temprature_maxPointer = temprature_max;
					*dtPointer = dt;
					*temperaturePointer = temperature;

					return;
				}

				/*
				 * flip it
				 */

				g[i*gsize+j] = 1 - g[i*gsize+j];

				count = CliqueCount(g,gsize);

				/*
				 * is it better and the i,j,count not taboo?
				 */
				 
				if((count < best_count) && 
					// !FIFOFindEdge(taboo_list,i,j))
					!FIFOFindEdgeCount(taboo_list,i,j,count))
				{
					best_count = count;
					best_i = i;
					best_j = j;

					// printf("ce size: %d, best_count: %d, best edge: (%d,%d), new color: %d\n",
					// gsize,
					// best_count,
					// best_i,
					// best_j,
					// g[best_i*gsize+best_j]);
				}

				/*
				 * flip it back
				 */

				g[i*gsize+j] = 1 - g[i*gsize+j];



			}

			if(best_count == BIGCOUNT) {
			printf("no best edge found, terminating\n");
			exit(1);
			}

			if (i == best_i)
			{
				g[best_i*gsize+best_j] = 1 - g[best_i*gsize+best_j];

				count = CliqueCount(g,gsize);
				// FIFOInsertEdge(taboo_list,best_i,best_j);
				FIFOInsertEdgeCount(taboo_list,best_i,best_j,count);

				printf("ce size: %d, best_count: %d, best edge: (%d,%d), new color: %d\n",
				gsize,
				best_count,
				best_i,
				best_j,
				g[best_i*gsize+best_j]);
			}
			

		}


		// if(best_count == BIGCOUNT) {
		// 	printf("no best edge found, terminating\n");
		// 	exit(1);
		// }
		
		// /*
		//  * keep the best flip we saw
		//  */
		//  g[best_i*gsize+best_j] = 1 - g[best_i*gsize+best_j];
		

		// /*
		//  * taboo this graph configuration so that we don't visit
		//  * it again
		//  */
		// count = CliqueCount(g,gsize);
		// // FIFOInsertEdge(taboo_list,best_i,best_j);
		// FIFOInsertEdgeCount(taboo_list,best_i,best_j,count);

		// printf("ce size: %d, best_count: %d, best edge: (%d,%d), new color: %d\n",
		// 	gsize,
		// 	best_count,
		// 	best_i,
		// 	best_j,
		// 	g[best_i*gsize+best_j]);

		/*
		 * rinse and repeat
		 */
	}

	

	*gPointer = *g;
	*new_gPointer = *new_g;
	*gsizePointer = gsize;
	*countPointer = count;
	*iPointer = i;
	*jPointer = j;
	*best_iPointer = best_i;
	*best_jPointer = best_j;
	*best_countPointer = best_count;
	taboo_listPointer = taboo_list;
	*temprature_maxPointer = temprature_max;
	*dtPointer = dt;
	*temperaturePointer = temperature;

	FIFODeleteGraph(taboo_list);

	//return(0);

}


int
main(int argc,char *argv[])
{
	int *g;
	int *new_g;
	int gsize;
	int count;
	int i;
	int j;
	int best_count;
	int best_i;
	int best_j;
	void *taboo_list;

	double temprature_max = 1000000;
	double dt = 1;
	double temperature = temprature_max;

	//best_count = BIGCOUNT;

	gsize = 90;
	g = (int *)malloc(gsize*gsize*sizeof(int));
	if(g == NULL) {
		exit(1);
	}

	/*
	 * make a fifo to use as the taboo list
	 */
	taboo_list = FIFOInitEdge(TABOOSIZE);
	if(taboo_list == NULL) {
		exit(1);
	}

	/*
	 * start out with all zeros
	 */

	 //memset(g,0,gsize*gsize*sizeof(int));

	 for (int i = 0; i < gsize; ++i)
	 {
	 	for (int j = 0; j < gsize; ++j)
	 	{
	 		int r = rand() % 10;
	 		if (r<5)
	 		{
	 			g[i*gsize+j] = 0;
	 		}
	 		else
	 		{
	 			g[i*gsize+j] = 1;
	 		}
	 	}
	 }
	 
	 PrintGraph(g, gsize);

	 while (gsize < 102)
	 {
	 	while (best_count>0)
		 {
		 	if (best_count > 5500)
			{
				tabooSearch(g, new_g, &gsize, &count, &i, &j, &best_count, &best_i, &best_j, 
					taboo_list, &temprature_max, &dt, &temperature);
			}

			printf("best count after taboo search is %d\n", best_count);

			if ( (best_count <= 5500 && temperature > 0) )
			{
				simulatedAnnealing(g, &gsize, &i, &j, &best_i, &best_j, &best_count,
				 taboo_list, &temprature_max, &dt, &temperature);
			}

			if (temperature <= 0)
			{
				return(0);
			}
			

		 }

		printf("Eureka!  Counter-example found!\n");
		PrintGraph(g,gsize);
		/*
		 * make a new graph one size bigger
		 */
		 new_g = (int *)malloc((gsize+1)*(gsize+1)*sizeof(int));
		 if(new_g == NULL)
		 	exit(1);
		/*
		 * copy the old graph into the new graph leaving the
		 * last row and last column alone
		 */
		 CopyGraph(g,gsize,new_g,gsize+1);

		/*
		 * zero out the last column and last row
		 */

		 new_g[i*(gsize+1) + gsize] = 0; // last column
		 new_g[gsize*(gsize+1) + i] = 0; // last row

		/*
		 * throw away the old graph and make new one the
		 * graph
		 */
		 free(g);
		 g = new_g;
		 gsize = gsize+1;

		/*
		 * reset the taboo list for the new graph
		 */
		taboo_list = FIFOResetEdge(taboo_list);

		/*
		 * keep going
		 */
		continue;

	 }

	return (0);

}
