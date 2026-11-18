// Read graph6 from stdin, decide antimagic labeling; print counterexamples.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int n, m, eu[256], ev[256], ord[256];
static uint64_t incBits[64];
static int ssum[64];
static uint64_t used;
static long long nodes, worst_nodes;
static int budget_abort;

static int all_distinct(void){
  for (int a=0;a<n;a++) for (int b=a+1;b<n;b++) if (ssum[a]==ssum[b]) return 0;
  return 1;
}

static int rec(int j){
  if (j==m) return all_distinct();
  nodes++;
  if (nodes > 40000000LL){ budget_abort=1; return 0; }
  int e = ord[j], u = eu[e], v = ev[e];
  uint64_t rem = (j+1 >= 64) ? 0ULL : ~((1ULL<<(j+1))-1ULL);
  for (int L=1; L<=m; L++){
    uint64_t bit = 1ULL<<(L-1);
    if (used & bit) continue;
    used |= bit; ssum[u]+=L; ssum[v]+=L;
    int ok = 1;
    for (int a=0;a<n && ok;a++)
      for (int b=a+1;b<n;b++)
        if (ssum[a]==ssum[b]){
          uint64_t ra = incBits[a]&rem, rb = incBits[b]&rem;
          if ((ra & ~rb)==0 && (rb & ~ra)==0){ ok=0; break; }
        }
    if (ok && rec(j+1)){ used&=~bit; ssum[u]-=L; ssum[v]-=L; return 1; }
    used&=~bit; ssum[u]-=L; ssum[v]-=L;
    if (budget_abort) return 0;
  }
  return 0;
}

int main(void){
  char line[8192];
  long long total=0, bad=0, undec=0;
  worst_nodes = 0;
  while (fgets(line, sizeof line, stdin)){
    if (line[0]=='#') continue;
    if (line[0]=='>') continue;
    // parse graph6
    int len = strlen(line); while (len && (line[len-1]=='\n'||line[len-1]=='\r')) line[--len]=0;
    if (!len) continue;
    unsigned char *p = (unsigned char*)line;
    if (p[0]==126){ fprintf(stderr,"n>63 unsupported\n"); return 1; }
    n = p[0]-63; p++;
    if (n < 1 || n > 32){ fprintf(stderr,"n out of range: %d\n", n); return 1; }
    int bitsNeeded = n*(n-1)/2;
    int byteCount = (bitsNeeded+5)/6;
    int adj[64][64]; memset(adj,0,sizeof adj);
    int pu[512], pv[512], np=0;
    for (int jj=1;jj<n;jj++) for (int ii=0;ii<jj;ii++){ pu[np]=ii; pv[np]=jj; np++; }
    int idx=0;
    for (int b=0;b<byteCount;b++){
      int val = p[b]-63;
      for (int k=5;k>=0;k--){
        int bit = (val>>k)&1;
        if (idx < np){ adj[pu[idx]][pv[idx]] = bit; adj[pv[idx]][pu[idx]] = bit; }
        idx++;
      }
    }
    m=0;
    for (int u=0;u<n;u++) for (int v=u+1;v<n;v++) if (adj[u][v]){ eu[m]=u; ev[m]=v; m++; }
    if (m==0){ continue; }
    // greedy edge order
    int deg[64]; memset(deg,0,sizeof deg);
    for (int e=0;e<m;e++){ deg[eu[e]]++; deg[ev[e]]++; }
    int avail[256]; for (int e=0;e<m;e++) avail[e]=e;
    int placed=0;
    while (placed<m){
      int best=-1,bestScore=-1;
      for (int i=0;i<m;i++){ int e=avail[i]; if (e<0) continue;
        int sc = deg[eu[e]]+deg[ev[e]];
        if (placed>0){ int last=ord[placed-1];
          if (eu[e]==eu[last]||eu[e]==ev[last]||ev[e]==eu[last]||ev[e]==ev[last]) sc+=1000; }
        if (sc>bestScore){ bestScore=sc; best=e; } }
      for (int i=0;i<m;i++) if (avail[i]==best) avail[i]=-1;
      ord[placed++]=best;
    }
    memset(incBits,0,sizeof incBits);
    for (int j=0;j<m;j++){ incBits[eu[ord[j]]] |= 1ULL<<j; incBits[ev[ord[j]]] |= 1ULL<<j; }
    memset(ssum,0,sizeof ssum); used=0; nodes=0; budget_abort=0;
    total++;
    int r = rec(0);
    if (nodes > worst_nodes) worst_nodes = nodes;
    if (budget_abort) undec++;
    else if (!r){ bad++; if (bad<=5) printf("COUNTEREXAMPLE %s\n", line); }
  }
  printf("graphs=%lld counterexamples=%lld undecided=%lld worst_nodes=%lld\n", total, bad, undec, worst_nodes);
  return 0;
}
