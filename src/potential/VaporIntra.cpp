#include "../md.hpp"
/*########################################################################################

-----compute intramolecular interaction-----

#######################################################################################*/
#define TOLERANCE 0.05
#define SMALL     0.001
#define SMALLER   0.00001
/**********************************Force calculation******************************************/
void
PotentialVaporIntra::compute(Variables *vars) {
	Molecule *vapors = vars->vapors.data();
	Bond_type *btypes_v = vars->btypes_v.data();
	Angle_type *ctypes_v = vars->ctypes_v.data();
	Dihedral_type *dtypes_v = vars->dtypes_v.data();

	double dx1, dy1, dz1, dx2, dy2, dz2, rsq1, rsq2, r1, r2, C, Cs, dtheta, tk, a, a11, a12, a22, f1[3], f3[3];

	double vb1x,vb1y,vb1z,vb2x,vb2y,vb2z,vb3x,vb3y,vb3z,vb2xm,vb2ym,vb2zm;
	double edihedral,ff2[3],ff4[3],ff1[3],ff3[3];
	double ax,ay,az,bx,by,bz,rasq,rbsq,rgsq,rg,rginv,ra2inv,rb2inv,rabinv;
	double df,df1,ddf1,fg,hg,fga,hgb,gaa,gbb;
	double dtfx,dtfy,dtfz,dtgx,dtgy,dtgz,dthx,dthy,dthz;
	double c,s,p_,sx2,sy2,sz2, m;

	double sb1,sb2,sb3,rb1,rb3,c0,b1mag2,b1mag,b2mag2;
	double b2mag,b3mag2,b3mag,ctmp,r12c1,c1mag,r12c2;
	double c2mag,sc1,sc2,s1,s12,p,pd;
	double a33,a13,a23;
	double s2,cx,cy,cz,cmag,dx,phi,si,siinv,sin2;
	vars->times.tvap-=omp_get_wtime();
	for(auto &I : vars->vapor_in){
		for (auto &b : vapors[I].bonds) {
			int i=b.atom1, j=b.atom2, type=(b.type);
			dx1 = vapors[I].inAtoms[i].qx - vapors[I].inAtoms[j].qx;
			dy1 = vapors[I].inAtoms[i].qy - vapors[I].inAtoms[j].qy;
			dz1 = vapors[I].inAtoms[i].qz - vapors[I].inAtoms[j].qz;
			rsq1 = (dx1 * dx1 + dy1 * dy1 + dz1 * dz1);
			r1 = sqrt(rsq1);
			double dr = (r1-btypes_v[type].coeff[1]);
			double rk = btypes_v[type].coeff[0] * dr;
			double force_bond_harmonic;
			force_bond_harmonic = -2.0*rk/r1;
			vapors[I].inAtoms[i].fx += force_bond_harmonic * dx1;
			vapors[I].inAtoms[i].fy += force_bond_harmonic * dy1;
			vapors[I].inAtoms[i].fz += force_bond_harmonic * dz1;
			vapors[I].inAtoms[j].fx -= force_bond_harmonic * dx1;
			vapors[I].inAtoms[j].fy -= force_bond_harmonic * dy1;
			vapors[I].inAtoms[j].fz -= force_bond_harmonic * dz1;
			if(vars->eflag) vars->U.Uvap+=rk*dr;
		}

		for (auto &c : vapors[I].angles) {
		    int i, j, k, type;
			i=c.atom1, j=c.atom2, k=c.atom3, type=c.type;
			dx1 = vapors[I].inAtoms[i].qx - vapors[I].inAtoms[j].qx;
			dy1 = vapors[I].inAtoms[i].qy - vapors[I].inAtoms[j].qy;
			dz1 = vapors[I].inAtoms[i].qz - vapors[I].inAtoms[j].qz;
			rsq1 = (dx1 * dx1 + dy1 * dy1 + dz1 * dz1);
			r1 = sqrt(rsq1);
			dx2 = vapors[I].inAtoms[k].qx - vapors[I].inAtoms[j].qx;
			dy2 = vapors[I].inAtoms[k].qy - vapors[I].inAtoms[j].qy;
			dz2 = vapors[I].inAtoms[k].qz - vapors[I].inAtoms[j].qz;
			rsq2 = (dx2 * dx2 + dy2 * dy2 + dz2 * dz2);
			r2 = sqrt(rsq2);
			C = dx1*dx2 + dy1*dy2 + dz1*dz2;
			C /= r1*r2;
			Cs = 1/(sqrt(1.0-C*C));
			dtheta = acos(C) - ctypes_v[type].coeff[1];
			tk = ctypes_v[type].coeff[0] * dtheta;
			a = -2.0 * tk * Cs;
			a11 = a*C / rsq1;
			a12 = -a / (r1*r2);
			a22 = a*C / rsq2;
			f1[0] = a11*dx1 + a12*dx2;
			f1[1] = a11*dy1 + a12*dy2;
			f1[2] = a11*dz1 + a12*dz2;
			f3[0] = a22*dx2 + a12*dx1;
			f3[1] = a22*dy2 + a12*dy1;
			f3[2] = a22*dz2 + a12*dz1;
			vapors[I].inAtoms[i].fx += f1[0];
			vapors[I].inAtoms[i].fy += f1[1];
			vapors[I].inAtoms[i].fz += f1[2];
			vapors[I].inAtoms[j].fx -= f1[0] + f3[0];
			vapors[I].inAtoms[j].fy -= f1[1] + f3[1];
			vapors[I].inAtoms[j].fz -= f1[2] + f3[2];
			vapors[I].inAtoms[k].fx += f3[0];
			vapors[I].inAtoms[k].fy += f3[1];
			vapors[I].inAtoms[k].fz += f3[2];
			if (vars->eflag) vars->U.Uvap+= tk*dtheta;
		}


		for (auto &d : vapors[I].dihedrals) {
			int i=d.atom1, j=d.atom2, k=d.atom3, l=d.atom4, type=d.type;
			// 1st bond
			vb1x = vapors[I].inAtoms[i].qx - vapors[I].inAtoms[j].qx;
			vb1y = vapors[I].inAtoms[i].qy - vapors[I].inAtoms[j].qy;
			vb1z = vapors[I].inAtoms[i].qz - vapors[I].inAtoms[j].qz;
			// 2nd bond
			vb2x = vapors[I].inAtoms[k].qx - vapors[I].inAtoms[j].qx;
			vb2y = vapors[I].inAtoms[k].qy - vapors[I].inAtoms[j].qy;
			vb2z = vapors[I].inAtoms[k].qz - vapors[I].inAtoms[j].qz;
			vb2xm = -vb2x;
			vb2ym = -vb2y;
			vb2zm = -vb2z;
			// 3rd bond
			vb3x = vapors[I].inAtoms[l].qx - vapors[I].inAtoms[k].qx;
			vb3y = vapors[I].inAtoms[l].qy - vapors[I].inAtoms[k].qy;
			vb3z = vapors[I].inAtoms[l].qz - vapors[I].inAtoms[k].qz;

			ax = vb1y*vb2zm - vb1z*vb2ym;
			ay = vb1z*vb2xm - vb1x*vb2zm;
			az = vb1x*vb2ym - vb1y*vb2xm;
			bx = vb3y*vb2zm - vb3z*vb2ym;
			by = vb3z*vb2xm - vb3x*vb2zm;
			bz = vb3x*vb2ym - vb3y*vb2xm;
			rasq = ax*ax + ay*ay + az*az;
			rbsq = bx*bx + by*by + bz*bz;
			rgsq = vb2xm*vb2xm + vb2ym*vb2ym + vb2zm*vb2zm;
			rg = sqrt(rgsq);
			rginv = ra2inv = rb2inv = 0.0;
			if (rg > 0) rginv = 1.0/rg;
			if (rasq > 0) ra2inv = 1.0/rasq;
			if (rbsq > 0) rb2inv = 1.0/rbsq;
			rabinv = sqrt(ra2inv*rb2inv);
			c = (ax*bx + ay*by + az*bz)*rabinv;
			s = rg*rabinv*(ax*vb3x + ay*vb3y + az*vb3z);

			df = 0.0;
			int J= dtypes_v[type].multi;

			for(int JJ=0;JJ<J;JJ++){
				int JJ5=JJ*5;
				p_=1.0;
				ddf1=df1=0.0;
				for (int loop=0; loop<dtypes_v[type].coeff[JJ5+1]; loop++){
					ddf1 = p_*c - df1*s;
					df1 = p_*s + df1*c;
					p_ = ddf1;
				}
				p_ = p_*dtypes_v[type].coeff[JJ5+3] + df1*dtypes_v[type].coeff[JJ5+4];
				df1 = df1*dtypes_v[type].coeff[JJ5+3] - ddf1*dtypes_v[type].coeff[JJ5+4];
				df1 *= -dtypes_v[type].coeff[JJ5+1];
				p_ += 1.0;
		        if(dtypes_v[type].coeff[1]==0){
		            p_=1.0+dtypes_v[type].coeff[JJ5+3];
		            df1=0.0;
		        }
				df += (-dtypes_v[type].coeff[JJ5] * df1);
				if (vars->eflag) vars->U.Uvap+= dtypes_v[type].coeff[JJ5] * p_;
			}

			fg = vb1x*vb2xm + vb1y*vb2ym + vb1z*vb2zm;
			hg = vb3x*vb2xm + vb3y*vb2ym + vb3z*vb2zm;
			fga = fg*ra2inv*rginv;
			hgb = hg*rb2inv*rginv;
			gaa = -ra2inv*rg;
			gbb = rb2inv*rg;
			dtfx = gaa*ax;
			dtfy = gaa*ay;
			dtfz = gaa*az;
			dtgx = fga*ax - hgb*bx;
			dtgy = fga*ay - hgb*by;
			dtgz = fga*az - hgb*bz;
			dthx = gbb*bx;
			dthy = gbb*by;
			dthz = gbb*bz;
			sx2 = df*dtgx;
			sy2 = df*dtgy;
			sz2 = df*dtgz;
			ff1[0] = df*dtfx;
			ff1[1] = df*dtfy;
			ff1[2] = df*dtfz;
			ff2[0] = sx2 - ff1[0];
			ff2[1] = sy2 - ff1[1];
			ff2[2] = sz2 - ff1[2];
			ff4[0] = df*dthx;
			ff4[1] = df*dthy;
			ff4[2] = df*dthz;
			ff3[0] = -sx2 - ff4[0];
			ff3[1] = -sy2 - ff4[1];
			ff3[2] = -sz2 - ff4[2];

			vapors[I].inAtoms[i].fx += ff1[0];
			vapors[I].inAtoms[i].fy += ff1[1];
			vapors[I].inAtoms[i].fz += ff1[2];
			vapors[I].inAtoms[j].fx += ff2[0];
			vapors[I].inAtoms[j].fy += ff2[1];
			vapors[I].inAtoms[j].fz += ff2[2];
			vapors[I].inAtoms[k].fx += ff3[0];
			vapors[I].inAtoms[k].fy += ff3[1];
			vapors[I].inAtoms[k].fz += ff3[2];
			vapors[I].inAtoms[l].fx += ff4[0];
			vapors[I].inAtoms[l].fy += ff4[1];
			vapors[I].inAtoms[l].fz += ff4[2];

		}
	}
	vars->times.tvap+=omp_get_wtime();
}
