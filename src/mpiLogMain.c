#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <math.h>

#define LENGTH     16384
#define EPSILON  0.1

MPI_Status status;

double When();

bool reduceBool(int ndims, int id, bool value)
{
    int not_participating = 0;
    int bit_mask = 1;
    bool final_value = value;
    bool new_value;
    int cur_dim;

    for(cur_dim = 0; cur_dim < ndims; cur_dim++) {
        if ((id & not_participating) == 0) {
            if ((id & bit_mask) != 0) {
                int dest_id = id ^ bit_mask;
                MPI_Send(&final_value, 1, MPI_CHAR, dest_id, 0, MPI_COMM_WORLD);// Send
            } else {
                int src_id = id ^ bit_mask;
                MPI_Recv(&new_value, 1, MPI_CHAR, src_id, 0, MPI_COMM_WORLD, &status);// Receive
                // printf("REDUCE: MPI task %d got %f from %d\n", id, new_value, src_id);
                final_value &= new_value;
            }
        }
        not_participating = not_participating ^ bit_mask;
        bit_mask <<=1;
    }
    return final_value;
}

bool broadcastBool(int ndims, int id, bool value) {
    int not_participating = (int)pow(2.0, ndims-1) - 1;
    int bit_mask = (int)pow(2.0, ndims - 1);
    bool new_value = value;
    int cur_dim;

    // printf("ndims: %d\n", ndims);

    for (cur_dim = 0; cur_dim < ndims; cur_dim++) {
        if ((id & not_participating) == 0) {
            if ((id & bit_mask) == 0) {
                int dest_id = id ^ bit_mask;
                MPI_Send(&new_value, 1, MPI_CHAR, dest_id, 0, MPI_COMM_WORLD);// Send
            }
            else {
                int src_id = id ^ bit_mask;
                MPI_Recv(&new_value, 1, MPI_CHAR, src_id, 0, MPI_COMM_WORLD, &status);// Receive
            }
        }
        not_participating >>= 1;
        bit_mask >>= 1;
    }

    return new_value;
}

void printPlate(float** plate, int cnt, int section_len, int iproc) {
    for (int i = 1; i < section_len + 1; i++) {
        printf("%d -> %d (%d): |", cnt, iproc, i);
        for (int j = 0; j < LENGTH; j++) {
            printf("%6.1f| ", plate[i][j]);
        }
        printf("\n");
    }
}

float calcCell(float** plate, int row, int col, int iproc) {
    float up = plate[row-1][col];
    float left = plate[row][col-1];
    float center = plate[row][col];
    float right = plate[row][col+1];
    float down = plate[row+1][col];
    float val = (up + left + right + down + (center * 4.0f)) / 8.0f;
    return val;
}

bool isCellSteady(float** plate, int row, int col) {
    float up = plate[row-1][col];
    float left = plate[row][col-1];
    float center = plate[row][col];
    float right = plate[row][col+1];
    float down = plate[row+1][col];
    return fabs(center - (up + left + right + down) / 4.0f) < EPSILON;
}

void main(int argc, char *argv[])
{
    float **curr_plate, **prev_plate, **tmp;
    int i, j;
    bool done, reallydone;
    int cnt = 0;
    int start, end;
    int section_len;

    double starttime;

    int nproc, iproc;

    MPI_Init(&argc, &argv);
    starttime = When();

    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &iproc);
    int ndims = (int)log2(nproc);
    // fprintf(stderr,"%d: Hello from %d of %d\n", iproc, iproc, nproc);
    
    /* Determine how much I should be doing and allocate the arrays*/
    section_len = LENGTH / nproc;
    curr_plate = malloc((section_len + 2) * sizeof(float*));
    prev_plate = malloc((section_len + 2) * sizeof(float*));

    for (i = 0; i < section_len + 2; i++) {
        curr_plate[i] = malloc(LENGTH * sizeof(float));
        prev_plate[i] = malloc(LENGTH * sizeof(float));
    }

    start = 1;
    end = section_len + 1;

    /* Initialize the cells */
    for (i = 0; i < section_len + 2; i++)
    {
        curr_plate[i][0] = prev_plate[i][0] = 0;
        for (j = 1; j < LENGTH - 1; j++) {
            curr_plate[i][j] = prev_plate[i][j] = 50;
        }
        curr_plate[i][LENGTH - 1] = prev_plate[i][LENGTH - 1] = 0;
    }

    /* Initialize the Boundaries */
    if (iproc == 0)
    {
        start = 2;
        for (i = 0; i < LENGTH; i++) {
            curr_plate[1][i] = prev_plate[1][i] = 0;
        }
    }
    if (iproc == nproc - 1)
    {
        end = section_len;
        for (i = 0; i < LENGTH; i++) {
            curr_plate[section_len][i] = prev_plate[section_len][i] = 100;
        }
    }

    /* Now run the relaxation */
    reallydone = false;
    for(cnt = 0; !reallydone; cnt++)
    {
        // if (iproc == 0) { printf("cnt = %d\n", cnt); }

        // printPlate(prev_plate, cnt, section_len, iproc);
        if (cnt > 500) { printf("cnt exceeded\n"); break; }
        /* First, I must get my neighbors boundary values */
        if (iproc != 0)
        {
            /* send up */
            MPI_Send(prev_plate[1], LENGTH, MPI_FLOAT, iproc - 1, 0, MPI_COMM_WORLD);
            /* receive up */
            MPI_Recv(prev_plate[0], LENGTH, MPI_FLOAT, iproc - 1, 0, MPI_COMM_WORLD, &status);
        }
        if (iproc != nproc - 1)
        {
            /* receive down */
            MPI_Recv(prev_plate[section_len + 1], LENGTH, MPI_FLOAT, iproc + 1, 0, MPI_COMM_WORLD, &status);
            /* send down */
            MPI_Send(prev_plate[section_len], LENGTH, MPI_FLOAT, iproc + 1, 0, MPI_COMM_WORLD);
        }

        // printPlate(curr_plate, cnt, section_len, iproc);

        /* Do the calculations */
        for (i = start; i < end; i++)
        {
            for (j = 1; j < LENGTH - 1; j++) {
                curr_plate[i][j] = calcCell(prev_plate, i, j, iproc);
            }
        }

        /* Check to see if we are done */
        done = 1;
        for (i = start; i < end; i++)
        {
            for (j = 1; j < LENGTH - 1; j++) {
                if (!isCellSteady(curr_plate, i, j))
                {
                    done = 0;
                    break;
                }
            }
        }

        /* Do a reduce to see if everybody is done */
        reallydone = reduceBool(ndims, iproc, done);
        reallydone = broadcastBool(ndims, iproc, reallydone);

        /* Swap the pointers */
        tmp = curr_plate;
        curr_plate = prev_plate;
        prev_plate = tmp;
    }

    /* print out the number of iterations to relax */
    if (iproc == 0) {
        printf("%d, %lf, ", cnt, When() - starttime);
    }
    MPI_Finalize();
}

/* Return the correct time in seconds, using a double precision number.       */
double When()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double) tp.tv_sec + (double) tp.tv_usec * 1e-6);
}
