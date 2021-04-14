/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>

//#include "reader.h"
//#include "parser.h"


int main(){
    int array[] = {-2,6,-1,3,-2,10,-5};
    int max = array[0];
    int l = 1;
    int cl = 1;
    int sum = array[0];
    for (int i = 1; i < 7; ++i){
//        max = sum + array[i] > sum ? (sum + array[i] > max ? sum + array[i] : max) : (array[i] > max ? array[i] : max);
        if(sum + array[i] > max){
            max = sum + array[i];
            sum+=array[i];
            l = cl+1;
            cl+=1;
        }else {
            sum+=array[i];
            cl+=1;
        }
    }

    printf("max : %d\nlength : %d\n",max,l);
        return 0;
}



///******************************************************************/
//
//int main(int argc, char *argv[]) {
////  if (argc <= 1) {
////    printf("parser: no input file.\n");
////    return -1;
////  }
//
//  if (compile("test\\example2.kpl") == IO_ERROR) {
//    printf("Can\'t read input file!\n");
//    return -1;
//  }
//
//  return 0;
//}
