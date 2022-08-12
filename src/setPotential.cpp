//------------------------------------------------------------------------
#include "md.hpp"
//------------------------------------------------------------------------


void
MD::setPotential(FLAG *flags){
//ion-ion
	IntraInter.clear();
	InterInter.clear();
	int flag=0;
	cout<<"\t|\tion\t|\tgas\t|\tvapor\t|"<<endl;
	if(flags->force_ters) {
		IntraInter.push_back(new PotentialTersoff());
		cout<<"ion\t|\tTersoff\t";
		flag=1;
	}
	if(flags->force_sw) {
		IntraInter.push_back(new PotentialSW());
		cout<<"ion\t|\tSW\t";
		flag=1;
	}
	if(flags->intra_AMBER) {
		IntraInter.push_back(new PotentialAMBER());
		cout<<"ion\t|\tAMBER\t";
		flag=1;
	}
	if(flags->force_born) {
		IntraInter.push_back(new PotentialBorn());	
		cout<<"ion\t|\tBMH\t";
		flag=1;
	}
	if(flag==0)	cout<<"ion\t|\tNO\t";


//ion-gas
	flag=0;
	if(flags->force_lj) {
		InterInter.push_back(new PotentialGasIon());
		cout<<"|\tL-J\t";
		flag=1;
	}
	if(flag==0)	cout<<"|\tNO\t";

//ion-vapor
	flag=0;
	if(flags->inter_vi) {
		InterInter.push_back(new PotentialVaporIon());
		cout<<"|\tL-J-C\t|"<<endl;
		flag=1;
	}
	if(flag==0)	cout<<"|\tNO\t|"<<endl;

//gas-ion(skip)
	cout<<"gas\t|\t-\t";

//gas-gas
	flag=0;
	if(flags->inter_gg) {
		InterInter.push_back(new PotentialGasGas());
		cout<<"|\tL-J\t";
		flag=1;
	}
	if(flag==0)	cout<<"|\tNO\t";


//gas-vapor
	flag=0;
	if(flags->inter_vg) {
		InterInter.push_back(new PotentialVaporGas());
		cout<<"|\tL-J\t|"<<endl;
		flag=1;
	}
	if(flag==0)	cout<<"|\tNO\t|"<<endl;


//vapor-ion-gas(skip)
	cout<<"vapor\t|\t-\t|\t-\t";


//vapor-vapor
	flag=0;
	if(flags->inter_vv) {
		InterInter.push_back(new PotentialVaporVapor());
		cout<<"|\tL-J-C\t|"<<endl;
		flag=1;
	}
	if(flag==0)	cout<<"|\tNO\t|"<<endl;


	if(flags->vapor_intra==1) {
		if(vaportype==2){
			IntraInter.push_back(new PotentialIntraTIP3P());
			cout<<"vepor intra -->TIP3P\t"<<endl;
		}
		else{
			IntraInter.push_back(new PotentialVaporIntra());
			cout<<"vepor intra -->\tAMBER"<<endl;
		}
	}

	if(flags->gas_intra==1) {
		IntraInter.push_back(new PotentialGasIntra());
		cout<<"gas intra -->ON\t"<<endl;
	}

	if(flags->force_ion_dipole) {
		InterInter.push_back(new PotentialIonDipole());
		cout<<"gas-ion inter -->\tIon induced"<<endl;
	}
	if(flags->efield) {
		InterInter.push_back(new PotentialEfield());
		cout<<"Efield -->\tON"<<endl;
	}


}

