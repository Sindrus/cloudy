#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "fifo.h"	/* for taboo list */


#define MAXSIZE (512)

#define TABOOSIZE (5000)
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


int simulatedAnnealing(int *g, int *gsize, int *best_i, int *best_j, 
        int *best_count, void *taboo_list, double *temprature_max, 
        double *dt, double *temperature )
{

    int count;

	printf("hello simulated annealing\n");

	for (int i = 0; i < *gsize; ++i)
	{
		for (int j = 0; j < *gsize; ++j)
		{

			if ( *best_count == 0 )
				return 0;

			count = CliqueCount( g, *gsize );

			printf("count is %d\n", count);
			printf("best count is %d\n", *best_count );

			int best_neighbour = g[ ( *best_i) * ( *gsize) + ( *best_j ) ];

			double q = (double)( count ) - (double)( *best_count );
			double p = min( 1.0, exp((double)-q / (double)( *temperature ) ) );

			double randomNumber = (double)( rand() % 10 ) / 10;

			printf("q is %f\n", q);
			printf("p is %f\n", p);
			printf("minus q is %f\n", -q);
			printf("temp is %f\n", *temperature);
			printf("random number is %f\n", randomNumber);

			if ((randomNumber > p) && !FIFOFindEdgeCount( taboo_list, i, j, count))
			{
				g[ ( *best_i )* ( *gsize ) + ( *best_j ) ] = 
                        1 - g[ ( *best_i )*( *gsize ) + ( *best_j ) ];
				printf("exploiting better results\n");

		/*		best_count = best_count;
				best_i = best_i;
				best_j = best_j;
*/
				FIFOInsertEdgeCount( taboo_list, *best_i, *best_j, count);
			}
			else
			{
				/*while (count >= 6000)
				{
					*count = CliqueCount( g, *gsize );
				}*/

                count = CliqueCount( g, *gsize );
				int rand1 = rand() % *gsize;
				int rand2 = rand() % *gsize;

				while ( FIFOFindEdgeCount( taboo_list, rand1, rand2, count ) )
				{
					rand1 = rand() % *gsize;
					rand2 = rand() % *gsize;
				}
				
				if ( !FIFOFindEdgeCount( taboo_list, rand1, rand2, count ) )
				{
					printf("selecting random neighbour\n");

					printf("%d\n", rand1);
					printf("%d\n", rand2);

					g[ rand1 * ( *gsize ) + rand2 ] = 
                            1 - g[ rand1 * ( *gsize ) + rand2 ];

					//count = CliqueCount(g, gsize);

					*best_count = count;
					i = rand1;
					j = rand2;
					*best_i = rand1;
					*best_j = rand2;

					FIFOInsertEdgeCount( taboo_list, *best_i, *best_j, count );
				}
			}

			if( *temperature - *dt <= 0)
			{
				*temperature = 0;
				printf("error: stopped before solution was found\n");
				return 0;
			}
			else
			{
				*temperature = *temperature - *dt;
			}

			printf("ce size: %d, best_count: %d, best edge: (%d,%d), new color: %d\n",
			*gsize,
			*best_count,
			*best_i,
			*best_j,
			g[ ( *best_i ) * ( *gsize ) + ( *best_j ) ] );
		}
	}
	return 0;
}

void tabooSearch( int *g, int gsize, int *best_count, void **taboo_list ){
    int local_best_i = -1;
    int local_best_j = -1;
    int local_best_count = BIGCOUNT;
    int count = CliqueCount( g, gsize );

    for( int i = 0; i < gsize; i++){
        for( int j = i + 1; j < gsize; j++){
            g[ i * gsize + j ] = 1 - g[ i * gsize + j ];
    
            count = CliqueCount( g, gsize );

            if( ( count < local_best_count ) && 
                    !FIFOFindEdgeCount( *taboo_list, i, j, count ) ){
                local_best_count = count;
                local_best_i = i;
                local_best_j = j;
            }
            g[ i * gsize + j ] = 1 - g[ i * gsize + j ];
        }
    }
    if( local_best_i != -1 && local_best_j != -1 ){
        g[ local_best_i * gsize + local_best_j ] =
                1 - g[ local_best_i * gsize + local_best_j ];
        *best_count = local_best_count;
        FIFOInsertEdgeCount( *taboo_list, local_best_i, local_best_j, local_best_count);
    }
    printf( "Flipped best bit: ( %d, %d ), count %d\n",
            local_best_i, local_best_j, local_best_count );
    return;
}


void find_ramsey(){

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
	double temprature_max = 1.0;
	double dt = 0.00001;
	double temperature = temprature_max;

	gsize = 40;
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

    srand( time( NULL ) );
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
	 
//	PrintGraph(g, gsize);

    while (gsize < 41)
    {
	 	while (best_count>0)
        {
//		 	if (best_count > 600)
//			{
				tabooSearch( g, gsize, &best_count, &taboo_list );
//			}

//			printf("best count after taboo search is %d\n", best_count);

/*			if (best_count <= 6000 && temperature > 0)
			{
                printf( "lol\n" );
                exit( 1 );
				simulatedAnnealing(g, &gsize, &best_i, &best_j, &best_count,
				 taboo_list, &temprature_max, &dt, &temperature);
			}
*/
			if (temperature < 0)
			{
				return;
			}
        }
        FIFODeleteGraph( taboo_list );

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
}

int
main(int argc,char *argv[])
{
    find_ramsey();
	return (0);
}