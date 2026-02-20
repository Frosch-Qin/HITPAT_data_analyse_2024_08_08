#pragma once

//test 8 digits residue number system (RNS)
//Xi= x mod Pi
//P=(19,17,13,11,7,5,3,2)
//x[0,9699689] 
//9699689=19*17*13*11*7*5*3*2
//it's simple in multiplication and addition
//convert int to rns
void output_rns(int x){
  
  int P[8]={19,17,13,11,7,5,3,2};
  int xi[8];
  cout << x << " = " ;
  for (int i=0; i<8; i++){
    xi[i]=x%P[i];
    cout<< "("<<xi[i] <<")";
  }
  cout<< "  " << endl;
}


void scan(){
  
  ofstream rnsFile;
  rnsFile.open("rnsFile.txt");
  cout.rdbuf(rnsFile.rdbuf());
  int P[8]={19,17,13,11,7,5,3,2};
  int xi[8];
 
  for (int x=0; x<65535; x++){
    cout << x << " = " ;
    for (int i=0; i<8; i++){
      xi[i]=x%P[i];
      cout<< "("<<xi[i] <<")";
    }
    cout<< "  " << endl;
  }
  
  rnsFile.close();

}
