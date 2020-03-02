#include <stdio.h>

void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	int temp;
	int i,j,i1,j1;
	int block = 4;
	for(i = 0; i < N; i += block){
		for(j = 0; j < M; j += block){
			for(i1 = i; i1 < i+block; i1++){
				for(j1 = j; j1 < j+block; j1++){
					temp = A[i1][j1];
					B[j1][i1] = temp;
				}
			}
		}
	}
}
int main(){
int M = 4;
int N = 4;

int A[N][M];
int B[M][N];
	int i,j,cnt = 0;
	for(i = 0; i < N; i++)
		for(j = 0; j < M; j++)
			A[i][j] = cnt++;
	transpose_submit(M,N,A,B);
	for(j = 0; j < M; j++){
		for(i = 0; i < N; i++){
			printf("%d ",B[j][i]);
		}
		printf("\n");
	}
	return 0;
}

