#include <math.h>

double d1, d2, d3, d4, A, ep;
void chebbpf_setup() {
      //Chebyshev 4 pole bandpass filter
      //filter characteristics
      double s = 100;      //samples per second
      double f1 = 7.0;     //upper 50% power point
      double f2 = 0.7;     //lower 50% power point
  
      //calculate cheb 4 pole bdp coeff
      //float time;
      int    n = 4;
      double a, a2, b, b2, u, su, cu;

      a = cos(M_PI*(f1+f2)/s)/cos(M_PI*(f1-f2)/s);
      a2 = a*a;
      b = tan(M_PI*(f1-f2)/s);
      b2 = b*b;
      u = log((1.0+sqrt(1.0+ep*ep))/ep);
      su = sinh(2.0*u/(double)n);
      cu = cosh(2.0*u/(double)n);

      double r, c;
      r = sin(M_PI*(2.0*1.0)/n)*su;
      c = cos(M_PI*(2.0*1.0)/n)*cu;
      c = r*r + c*c;
      s = b2*c + 2.0*b*r + 1.0;
      A = b2/(4.0*s); 
      d1 = 4.0*a*(1.0+b*r)/s;
      d2 = 2.0*(b2*c-2.0*a2-1.0)/s;
      d3 = 4.0*a*(1.0-b*r)/s;
      d4 = -(b2*c - 2.0*b*r + 1.0)/s;
      ep = 2.0/ep; 
  }

  double red_w0, red_w1, red_w2, red_w3, red_w4;
  double red_bpf(double input) {
      red_w0 = d1*red_w1 + d2*red_w2 + d3*red_w3 + d4*red_w4 + input;
      input = A*(red_w0 - 2.0*red_w2 + red_w4);
      red_w4 = red_w3;
      red_w3 = red_w2;
      red_w2 = red_w1;
      red_w1 = red_w0;
      return(ep*input);
  }

  double ir_w0, ir_w1, ir_w2, ir_w3, ir_w4;
  double ir_bpf(double input) {
      ir_w0 = d1*ir_w1 + d2*ir_w2 + d3*ir_w3 + d4*ir_w4 + input;
      input = A*(ir_w0 - 2.0*ir_w2 + ir_w4);
      ir_w4 = ir_w3;
      ir_w3 = ir_w2;
      ir_w2 = ir_w1;
      ir_w1 = ir_w0;
      return(ep*input);
  }


  //peak extraction
  #define PEAK_NUM   5
  #define PEAK_WIN  32
  #define PEAK_SKIP  4
  float red_peak_adata[PEAK_WIN];
  float red_peak_tdata[PEAK_WIN];
  float red_peak_amp[PEAK_NUM];
  float red_peak_time[PEAK_NUM];
  float red_peak_amp_avg;
  float red_peak_amp_time;
  int red_peak_data_ptr = 0;
  int red_peak_ptr = 0;

void red_peak(float input, float time) {
    int hit, scan, pass;
    float peak_amp, peak_time;
    red_peak_adata[red_peak_data_ptr] = input;
    red_peak_tdata[red_peak_data_ptr] = time;
    pass = 1; //scan through amplitude array to see in middle value id the largest
    for(int i = 0; i < PEAK_WIN; i = i + PEAK_SKIP) {
        if(red_peak_adata[(red_peak_data_ptr + PEAK_WIN/2)%PEAK_WIN] > red_peak_adata[i]) pass = 0;
    }  
    peak_amp = red_peak_adata[(red_peak_data_ptr + PEAK_WIN/2)%PEAK_WIN];
    peak_time= red_peak_tdata[(red_peak_data_ptr + PEAK_WIN/2)%PEAK_WIN];
    if (pass == 1) {
        printf("peak found at %.2f of %.2f\n", peak_time, peak_amp);
    }
    red_peak_ptr = (++red_peak_ptr)%PEAK_WIN;
}
