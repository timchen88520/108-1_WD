#include<stdio.h>
#include<stdlib.h>

int determine(int game[][3]){
	int check[8]={0};
	int flag=0;

	for(int i=0;i<3;i++){
		check[0]+=game[0][i];
		check[1]+=game[1][i];
		check[2]+=game[2][i];
	}

	for(int i=0;i<3;i++)
		check[i+3]=game[0][i]+game[1][i]+game[2][i];



	check[6]=game[0][0]+game[1][1]+game[2][2];
	check[7]=game[2][0]+game[1][1]+game[0][2];

	for(int i=0;i<8;i++){
		if(check[i]==3){
			flag=1;
			return 1;
			break;
		}
		else if(check[i]==-3){
			flag=1;
			return 2;
			break;
		}

	}
	if(flag==0) return 0;

}
void insert(int game[][3]){
  	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++){
			int num;
			scanf("%d",&num);
			game[i][j]=num;
		}
}
int main(){
	int game[3][3];
  int num;

  insert(game);
	num=determine(game);
  printf("%d\n",num);
}
