#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "fifo.h"	/* for taboo list */
#include "c_api.h"

#define MAXSIZE (512)
#define MAXGRAPHSIZE ( 101 )

#define TABOOSIZE (500)
#define BIGCOUNT (9999999)

#define min(a,b) (((a) < (b)) ? (a) : (b))


struct HistoryStep{
    int i;
    int j;
};

struct History{
    size_t size;
    struct HistoryStep *hs;
    int stepIndex;
};

void initHistory( struct History *h ){
    h->size = 200;
    h->stepIndex = 0;
    h->hs = malloc( h->size*2 );
}

void add_to_history( struct History *h, int i, int j ){
    struct HistoryStep hs;
    hs.i = i;
    hs.j = j;
    h->hs[ h->stepIndex ] = hs;
    h->stepIndex = ( h->stepIndex + 1 ) % h->size;
}

void clean_history( struct History *h ){
    free( h->hs );
}

struct HistoryStep* get_last_step( struct History *h ){
    struct HistoryStep *hs = &h->hs[ h->stepIndex ];
    h->stepIndex = ( h->stepIndex - 1 ) % h->size;
    return hs;
}

/*
 * PrintGraph
 *
 * prints in the right format for the read routine
 */
void PrintGraph(int *g, int gsize)
{
	fprintf(stdout,"%d\n",gsize);

	for(int i=0; i < gsize; i++)
	{
		for(int j=0; j < gsize; j++)
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

	if( n_gsize < o_gsize )
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

int CliqueCount(int *g, int gsize ) {
    int i;
    int j;
    int k;
    int l;
    int m;
    int n;
    int count=0;
    int sgsize = 6;
    
    for(i=0;i < gsize-sgsize+1; i++){
	    for(j=i+1;j < gsize-sgsize+2; j++){
	        for(k=j+1;k < gsize-sgsize+3; k++) {
		        if((g[i*gsize+j] == g[i*gsize+k]) && 
                        (g[i*gsize+j] == g[j*gsize+k])){
		            for(l=k+1;l < gsize-sgsize+4; l++) { 
			            if((g[i*gsize+j] == g[i*gsize+l]) && 
			                    (g[i*gsize+j] == g[j*gsize+l]) && 
			                    (g[i*gsize+j] == g[k*gsize+l])){
			                for(m=l+1;m < gsize-sgsize+5; m++) {
				                if((g[i*gsize+j] == g[i*gsize+m]) && 
				                        (g[i*gsize+j] == g[j*gsize+m]) &&
                                        (g[i*gsize+j] == g[k*gsize+m]) && 
				                        (g[i*gsize+j] == g[l*gsize+m])) {
					                for(n=m+1; n < gsize-sgsize+6; n++){
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

/*
 * Made a small improvement on CliqueCount when we don't need to count all.
 */
int CliqueCountImp(int *g, int gsize, int current_best_count ) {
    int i;
    int j;
    int k;
    int l;
    int m;
    int n;
    int count=0;
    int sgsize = 6;
    
    for(i=0;i < gsize-sgsize+1; i++){
        if( (current_best_count != -1) && ( count > current_best_count ) )
            return BIGCOUNT;
	    for(j=i+1;j < gsize-sgsize+2; j++){
	        for(k=j+1;k < gsize-sgsize+3; k++) {
		        if((g[i*gsize+j] == g[i*gsize+k]) && 
                        (g[i*gsize+j] == g[j*gsize+k])){
		            for(l=k+1;l < gsize-sgsize+4; l++) { 
			            if((g[i*gsize+j] == g[i*gsize+l]) && 
			                    (g[i*gsize+j] == g[j*gsize+l]) && 
			                    (g[i*gsize+j] == g[k*gsize+l])){
			                for(m=l+1;m < gsize-sgsize+5; m++) {
				                if((g[i*gsize+j] == g[i*gsize+m]) && 
				                        (g[i*gsize+j] == g[j*gsize+m]) &&
                                        (g[i*gsize+j] == g[k*gsize+m]) && 
				                        (g[i*gsize+j] == g[l*gsize+m])) {
					                for(n=m+1; n < gsize-sgsize+6; n++){
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

				FIFOInsertEdgeCount( taboo_list, *best_i, *best_j, count);
			}
			else
			{

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

			printf("ce size: %d, best_count: %d, best edge: (%d,%d),\t new color: %d\n",
			*gsize,
			*best_count,
			*best_i,
			*best_j,
			g[ ( *best_i ) * ( *gsize ) + ( *best_j ) ] );
		}
	}
	return 0;
}

void fastTabooSearch( int *g, int gsize, int *best_count, void *taboo_list,
                    struct History *h ) {
	int local_best_i = -1;
    int local_best_j = -1;
    int local_best_count = BIGCOUNT;
    int count = CliqueCount( g, gsize );

    for( int i = 0; i < gsize; i++){
        for( int j = i + 1; j < gsize; j++){
            g[ i * gsize + j ] = 1 - g[ i * gsize + j ];
            count = CliqueCountImp( g, gsize, local_best_count );
            if( ( count < local_best_count ) && 
                    //!FIFOFindEdgeCount( taboo_list, i, j, count ) ){
                    !FIFOFindEdge( taboo_list, i, j ) ){
                local_best_count = count;
                local_best_i = i;
                local_best_j = j;
            }
            g[ i * gsize + j ] = 1 - g[ i * gsize + j ];
        }

        if( ( local_best_i > -1 ) && ( local_best_j > -1 ) && 
                ( local_best_count <= *best_count ) ){
        	g[ local_best_i * gsize + local_best_j ] =
                1 - g[ local_best_i * gsize + local_best_j ];

        	*best_count = local_best_count;
        	//FIFOInsertEdgeCount( taboo_list, local_best_i, local_best_j, local_best_count);
        	FIFOInsertEdge( taboo_list, local_best_i, local_best_j );

        	//add_to_history( h, local_best_i, local_best_j );

        	printf( "Flipped best bit: (%3d,%3d ), count %d, gsize: %d, new bit: %d\n",
                local_best_i, local_best_j, local_best_count, gsize, 
                g[ local_best_i * gsize + local_best_j ] );
//        printf( "new value: %d\n", g[ local_best_i * gsize + local_best_j ] );
//        PrintGraph( g, gsize );
    	} else {
        
        /*struct HistoryStep *hs = get_last_step( h );
        printf( "undo last step, wrong way. Last step: (%3d,%3d )\n",
                hs->i, hs->j );
        g[ hs->i * gsize + hs->j ] = 1 - g[ hs->i * gsize + hs->j ];
        *best_count = CliqueCount( g, gsize );*/

        srand( time( NULL ) );
        int i = rand()%gsize;
        int j = rand()%gsize;
        g[ i * gsize + j ] = 1 - g[ i * gsize + j ];
        *best_count = CliqueCount( g, gsize );
    }

    }
}


void tabooSearch( int *g, int gsize, int *best_count, void *taboo_list,
                    struct History *h ){
    int local_best_i = -1;
    int local_best_j = -1;
    int local_best_count = BIGCOUNT;
    int count = CliqueCount( g, gsize );

    for( int i = 0; i < gsize; i++){
        for( int j = i + 1; j < gsize; j++){
            g[ i * gsize + j ] = 1 - g[ i * gsize + j ];
            count = CliqueCountImp( g, gsize, local_best_count );
            if( ( count < local_best_count ) && 
                    //!FIFOFindEdgeCount( taboo_list, i, j, count ) ){
                    !FIFOFindEdge( taboo_list, i, j ) ){
                local_best_count = count;
                local_best_i = i;
                local_best_j = j;
            }
            g[ i * gsize + j ] = 1 - g[ i * gsize + j ];
        }
    }

    if( ( local_best_i > -1 ) && ( local_best_j > -1 ) && 
                ( local_best_count <= *best_count ) ){
        g[ local_best_i * gsize + local_best_j ] =
                1 - g[ local_best_i * gsize + local_best_j ];

        *best_count = local_best_count;
        //FIFOInsertEdgeCount( taboo_list, local_best_i, local_best_j, local_best_count);
        FIFOInsertEdge( taboo_list, local_best_i, local_best_j );

        //add_to_history( h, local_best_i, local_best_j );

        printf( "Flipped best bit: (%3d,%3d ), count %d, gsize: %d, new bit: %d\n",
                local_best_i, local_best_j, local_best_count, gsize, 
                g[ local_best_i * gsize + local_best_j ] );
//        printf( "new value: %d\n", g[ local_best_i * gsize + local_best_j ] );
//        PrintGraph( g, gsize );
    }else{
        
        /*struct HistoryStep *hs = get_last_step( h );
        printf( "undo last step, wrong way. Last step: (%3d,%3d )\n",
                hs->i, hs->j );
        g[ hs->i * gsize + hs->j ] = 1 - g[ hs->i * gsize + hs->j ];
        *best_count = CliqueCount( g, gsize );*/

        srand( time( NULL ) );
        int i = rand()%gsize;
        int j = rand()%gsize;
        g[ i * gsize + j ] = 1 - g[ i * gsize + j ];
        *best_count = CliqueCount( g, gsize );
    }
}


void find_ramsey(){
	int *g;
	int *new_g;
	int gsize;
	int count;
	int best_count;
    long graphId;
	void *taboo_list;
    struct History h;
    initHistory( &h );
	double temprature_max = 1.0;
	double dt = 0.00001;
	double temperature = temprature_max;
    time_t last_sync;

	gsize = 40;
	g = malloc( gsize * gsize * sizeof( int ) );
    new_g = malloc( 1 );
	if(g == NULL) {
		exit(1);
	}

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
	last_sync = time( NULL );
    while ( 1 ){
        fetch_new_graph( &g, &gsize, &graphId );
        while ( gsize < MAXGRAPHSIZE ){
            printf( "start graph: \n" );
            PrintGraph( g, gsize );
            best_count = CliqueCount( g, gsize );
           	fastTabooSearch( g, gsize, &best_count, taboo_list, &h );
            while ( best_count > 0 )
            {
                tabooSearch( g, gsize, &best_count, taboo_list, &h );
                if( difftime( time( NULL ), last_sync ) > 60 ){
                    printf( "sync\n" );
                    last_sync = time( NULL );
                    save_graph( graphId, gsize, g, 0 );
                }
            }

            printf("Counter-example found!\n");
            PrintGraph( g, gsize );
            if( gsize == 99 ){
                save_graph( graphId, gsize, g, 1 );
                exit( 0 );
            }
            
            new_g = (int *)malloc( ( gsize+1 ) * ( gsize+1 ) * sizeof( int ) );
            if(new_g == NULL)
                exit(1);
            PrintGraph( g, gsize );
            CopyGraph( g, gsize , new_g, gsize + 1 );

			for( int i=0; i < (gsize+1); i++){
				new_g[i*(gsize+1) + gsize] = 0;
				new_g[gsize*(gsize+1) + i] = 0;
			}

            free( g );
            g = new_g;

            taboo_list = FIFOResetEdge( taboo_list );
            gsize = gsize + 1;

            printf( "starting over on size %d\n", gsize );

        }
    }
    free( new_g );
    FIFODeleteGraph( taboo_list );
    clean_history( &h );
}

int
main(int argc,char *argv[])
{
    find_ramsey();
	return (0);
}
