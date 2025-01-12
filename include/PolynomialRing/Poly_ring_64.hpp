//code for polynomial ring whose coefficients are at most 64 bits integers. 
#include <iostream>
using namespace std;

extern intel::hexl::NTT* g_ntt;

typedef vector<int64_t> polynomial;

// Copy one polynomial
polynomial copy(const polynomial & p) {
  // INPUT: polynomial p
  // OUTPUT: a copy of p
  polynomial ans(p);
  return ans;
}

// Print polynomial
 void print_polynomial(const polynomial & p) {
  // INPUT: polynomial p
  // OUTPUT: print on screen

   cout << "exp: " << p[0] << ": ";
  for (int i = (int)p[0]+1; i > 1; --i) {
    cout << p[i] << "x^" << i-1 << "+";
  }
  cout << p[1] << endl;
}

// polynomial a mod q
// also modify a to ensure that the highest term of a isn't 0
inline void modq_poly(polynomial &a, int64_t q) {
  
  // INPUT: polynomial a, modulus q
  // OUTPUT: (modified) a mod q, stored in a

  register int tempexp = (int)a[0];
  int index = 1;
  for (register int i = 1; i <= tempexp+1; ++i) {

    while (a[i]<0) {
      a[i] += q;
    }

    while (a[i]>=q) {
      a[i] -= q;
    }

    if(a[i] >= q/2){
      a[i] -=q;
    }

    if (a[i] != 0) {
      index = i;
    }
  }
  a[0] = (int64_t)index-1;
  
  // modify a to ensure that the highest term of a isn't 0
  auto iter = a.erase(a.begin() + index+1, a.end());
}

// Addition between polynomials
 extern void add_poly(polynomial &a, const polynomial & b, int64_t q) {
  // INPUT: polynomials a and b, modulus q
  // OUTPUT: (a + b) mod q, stored in a
  if (a[0] < b[0]) {
    for (register int i = 1; i <= (int)a[0]+1; ++i) {
      a[i] += b[i];
    }
    a.insert(a.end(), b.begin() + a[0] + 2, b.end());
    a[0] = b[0];
  }
  else {
    for (register int i = 1; i <= (int)b[0] + 1; ++i) {
      a[i] += b[i];
    }
  }

  // mod q
  modq_poly(a, q);

}

 // Scaling
 extern void multi_scale_poly(int64_t t, polynomial &a, int64_t q) {
  // INPUT: scaler t, polynomial a, modulus q
  // OUTPUT: t*a mod q, stored in a

  for (register int i = 1; i <= a[0]+1; ++i) {
    a[i] *= t;
  }
  modq_poly(a, q);
}

// Compute a mod (x^n+1)
 void div_poly(polynomial &a, int n, int64_t q) {
  // INPUT: polynomial a, RING parameters n,q
  // OUTPUT: a mod (x^n+1), stored in a

  if (a[0] < n){
    return;
  }
  else {
    for (register int i = (int)a[0]+1; i > n ; --i) {
      a[i - n] -= a[i];
      a[i] = 0;
    }
    a[0] = (int64_t)n - 1;
    modq_poly(a, q);
  }
}


//quick_mul_poly for old LUT
 polynomial quick_mul_poly(int n, int64_t q, int a, polynomial const& b) {
   int index = 1;
   if (a < 0) {
     while (a < 0) {
       a += n;
       index *= -1;
     }
   }
   else {
     if(a>n){
       a -= n;
       index = -1;
     }
   }

   polynomial ans(b[0] + 2 + a, 0);
   ans[0] = b[0] + a;
   for (register int i = 1; i <= b[0]+1; ++i) {
     ans[i + a] = index * b[i];
     ans[i] -= b[i];
   }
   div_poly(ans, n, q);
   return ans;
 }



// NTT multiplication of two polynomials
polynomial NTTMul( const polynomial& aa, const polynomial& bb,int n, int64_t q) {
  vector<uint64_t> a(n),b(n);
  vector<uint64_t> c(n);
  uint64_t modulus = (uint64_t)q;

  for(int k=0;k<n;++k){
    if(k>aa[0]){
      a[k]=0;
    }
    else if(aa[k+1]>=0) a[k]=(uint64_t)aa[k+1];
    else if(aa[k+1]<0) a[k]=(uint64_t)(aa[k+1]+q);
  }
  for(int k=0;k<n;++k){
    if(k>bb[0]){
      b[k]=0;
    }
    else if(bb[k+1]>=0) b[k]=(uint64_t)bb[k+1];
    else if(bb[k+1]<0) b[k]=(uint64_t)(bb[k+1]+q);
  
  }
  
  //intel::hexl::NTT ntt(n, modulus);
 //ntt(a) and ntt(b)
  g_ntt->ComputeForward(a.data(), a.data(), 1, 1);
  g_ntt->ComputeForward(b.data(), b.data(), 1, 1);

  //c=ntt(a)*ntt(b)
  intel::hexl::EltwiseMultMod(c.data(), a.data(), b.data(), c.size(), modulus, 1);


  //intt(c)
  g_ntt->ComputeInverse(c.data(), c.data(), 1, 1);
  
  //vector<uint64_t> c -> polynomial cc
    
    polynomial cc(n+1);
    cc[0]=n-1;
    for(int k=0;k<n;++k){
        cc[k+1]=(int64_t)c[k];
        if(cc[k+1]>=q/2){
          cc[k+1] -= q;
        }
    }
    modq_poly(cc,q);
    return cc;
}

// NTT multiplication of two polynomials
vector<uint64_t> NTTMul_u( const polynomial& aa, const polynomial& bb,int n, int64_t q) {
  vector<uint64_t> a(n),b(n);
 // vector<uint64_t> c(n);
  uint64_t modulus = (uint64_t)q;

  for(int k=0;k<n;++k){
    if(k>aa[0]){
      a[k]=0;
    }
    else if(aa[k+1]>=0) a[k]=(uint64_t)aa[k+1];
    else if(aa[k+1]<0) a[k]=(uint64_t)(aa[k+1]+q);
  }
  for(int k=0;k<n;++k){
    if(k>bb[0]){
      b[k]=0;
    }
    else if(bb[k+1]>=0) b[k]=(uint64_t)bb[k+1];
    else if(bb[k+1]<0) b[k]=(uint64_t)(bb[k+1]+q);
  
  }
  
  //intel::hexl::NTT ntt(n, modulus);
 //ntt(a) and ntt(b)
  g_ntt->ComputeForward(a.data(), a.data(), 1, 1);
  g_ntt->ComputeForward(b.data(), b.data(), 1, 1);

  //c=ntt(a)*ntt(b)
  intel::hexl::EltwiseMultMod(a.data(), a.data(), b.data(), a.size(), modulus, 1);


  //intt(c)
  g_ntt->ComputeInverse(a.data(), a.data(), 1, 1);
  
  //vector<uint64_t> c -> polynomial cc
  /*  
    polynomial cc(n+1);
    cc[0]=n-1;
    for(int k=0;k<n;++k){
        cc[k+1]=(int64_t)c[k];
        if(cc[k+1]>=q/2){
          cc[k+1] -= q;
        }
    }
    modq_poly(cc,q);
    return cc;
    */
  return a;
}


//template <size_t N, size_t qNum, enum NTTSelector NTT, enum ImplSelector Impl>
 void XA_minus_1_2048(int n, uint64_t modulus, int a, vector<uint64_t> &xa){
  //uint64_t modulus = q;
  int index=1;
  for(int i=0;i<n;++i){
      xa[i]=0;
    }
  if(a == 0){
    return;
  }
  else if(a >= n){
    while(a>=n){
      a -= n;
      index *= -1;
    }
    if(index == -1){
      xa[a]=modulus-1;
      if (xa[0]==0) {
        xa[0]=modulus-1;
      } else {
        xa[0] -= 1;
      }
    }
    else if(index == 1){
      xa[a]=1;
      if (xa[0]==0) {
        xa[0]=modulus-1;
      } else {
        xa[0] -= 1;
      }
    }
    
    return;
  }
  else if(a<0){
    while(a<0){
      a+=n;
      index *= -1;
    }
    if(index == -1){
      xa[a]=modulus-1;
      if (xa[0]==0) {
        xa[0]=modulus-1;
      } else {
        xa[0] -= 1;
      }
    }
    else if(index == 1){
      xa[a]=1;
      if (xa[0]==0) {
        xa[0]=modulus-1;
      } else {
        xa[0] -= 1;
      }
    }
    return;
  }
  else{
    xa[a]=1;
    xa[0]=modulus-1;
    return;
  }
 }

// Extend polynomial w.r.t. base b: 
// Convert one polynomial to k/logb polynomials with coeff 0 or b-1
 vector<polynomial> bit_poly(int k, const polynomial & p, int64_t q, int64_t b, int logb) {
  // INPUT: k is about log(q), polynomial p, modulus q, base b, log(b)
  // OUTPUT: a vector of k/log(b) polynomials

  // initialize
  polynomial a((int)p[0] + 2, 0);
  a[0] = p[0];
  vector<polynomial> ans(k/logb,a);

  // fulfill ans slot by slot
  for (register int i = 1; i <= p[0]+1; ++i) {
    int64_t tempcoeff= p[i];
    while (tempcoeff < 0) {
      tempcoeff = tempcoeff + q;
    }
    
    int index = 0;
    while (tempcoeff != 0) {
      int t = tempcoeff % b;
      if (t> 0) {
        ans[index][i] = t;
        tempcoeff -= ans[index][i];
      }
      else {
        ans[index][i] = 0;
      }
      tempcoeff /= (b);
      index++;
    }
  }
  
  return ans;
}

 vector<vector<uint64_t>> bit_poly_u(int k, const vector<uint64_t> & p,int n, int64_t q, int64_t b, int logb) {
  // INPUT: k is about log(q), polynomial p, modulus q, base b, log(b)
  // OUTPUT: a vector of k/log(b) polynomials

  vector<vector<uint64_t>> ans(k/logb,vector<uint64_t>(n,0));
  //uint64_t bb = (uint64_t)b;

  // fulfill ans slot by slot
  for (register int i = 0; i < n; ++i) {
    int64_t tempcoeff= (int64_t)p[i];
   // if ((tempcoeff - p[i])!=0) cout << "err in uint and int" << endl;
    /*
    while (tempcoeff < 0) {
      tempcoeff = tempcoeff + q;
    }
    */
    int index = 0;
    while (tempcoeff != 0) {
      int64_t t = tempcoeff % b;
      if (t> 0) {
        ans[index][i] = (uint64_t)t;
        tempcoeff -= t;
      }
      else {
        ans[index][i] = 0;
      }
      tempcoeff /= (b);
      index++;
    }
  }
  
  return ans;
}