// Exhaustive check: antimagic labeling of connected graphs (Hartsfield-Ringel).
// usage: antimagic n shard nshards [count]
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static int n, m;
static int eu[64], ev[64];
static uint64_t incBits[16];
static int ord[64];
static int ssum[16];
static uint64_t used;
static long long nodes, budget = 4000000LL;
static long long worst_nodes = 0; static long long worst_mask = -1;
static int aborted;

static int all_distinct(void){
  for (int a=0;a<n;a++) for (int b=a+1;b<n;b++) if (ssum[a]==ssum[b]) return 0;
  return 1;
}

static int rec(int j){
  if (j==m) return all_distinct();
  if (++nodes > budget){ aborted=1; return 0; }
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
    if (ok && rec(j+1)) { used &= ~bit; ssum[u]-=L; ssum[v]-=L; return 1; }
    used &= ~bit; ssum[u]-=L; ssum[v]-=L;
    if (aborted) return 0;
  }
  return 0;
}

static long long cnt_rec(int j){
  if (j==m) return all_distinct() ? 1 : 0;
  if (++nodes > budget){ aborted=1; return 0; }
  int e = ord[j], u = eu[e], v = ev[e];
  uint64_t rem = (j+1 >= 64) ? 0ULL : ~((1ULL<<(j+1))-1ULL);
  long long total = 0;
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
    if (ok) total += cnt_rec(j+1);
    used &= ~bit; ssum[u]-=L; ssum[v]-=L;
    if (aborted) return total;
  }
  return total;
}

int main(int argc, char**argv){
  if (argc < 4){ fprintf(stderr,"usage: %s n shard nshards [count]\n", argv[0]); return 1; }
  n = atoi(argv[1]); int shard = atoi(argv[2]), nsh = atoi(argv[3]);
  int docount = (argc > 4 && argv[4][0]=='c');
  if (n < 1 || n > 9){ fprintf(stderr,"n must be 1..8\n"); return 1; }

  int ne = 0;
  for (int u=0;u<n;u++) for (int v=u+1;v<n;v++){ eu[ne]=u; ev[ne]=v; ne++; }
  m = ne;
  long long total = 1LL<<m;

  long long conn=0, anti=0, undec=0, labelings=0;
  for (int u=0;u<16;u++){ incBits[u]=0; }
  long long block = (total + nsh - 1) / nsh;
  long long lo = (long long)shard * block;
  long long hi = lo + block; if (hi > total) hi = total;
  for (long long mask=lo; mask<hi; mask++){
    // connectivity (bitset BFS)
    uint32_t reach = 1u, frontier = 1u;
    while (frontier){
      uint32_t nxt = 0;
      for (int e=0;e<m;e++) if ((mask>>e & 1LL) && ((frontier>>eu[e] & 1u) || (frontier>>ev[e] & 1u)))
        nxt |= (1u<<eu[e]) | (1u<<ev[e]);
      nxt &= ~reach; reach |= nxt; frontier = nxt;
    }
    if (n==1) reach = 1u;
    if (reach != (uint32_t)((1u<<n)-1)) continue;
    conn++;

    // build edge order
    int deg[16]; memset(deg,0,sizeof deg);
    int elist[64], cnt=0;
    for (int e=0;e<m;e++) if (mask>>e & 1LL){ elist[cnt++]=e; deg[eu[e]]++; deg[ev[e]]++; }
    // greedy: densest edge first, then edges sharing a vertex with placed ones
    int placed = 0; int avail[64]; memcpy(avail, elist, sizeof(int)*cnt);
    while (placed < cnt){
      int best=-1, bestScore=-1;
      for (int i=0;i<cnt;i++){
        int e = avail[i]; if (e<0) continue;
        int score = deg[eu[e]] + deg[ev[e]];
        if (placed>0){ int last = ord[placed-1];
          if (eu[e]==eu[last]||eu[e]==ev[last]||ev[e]==eu[last]||ev[e]==ev[last]) score += 100; }
        if (score > bestScore){ bestScore=score; best=e; }
      }
      for (int i=0;i<cnt;i++) if (avail[i]==best) avail[i]=-1;
      ord[placed++] = best;
    }
    for (int u=0;u<n;u++) incBits[u]=0;
    for (int j=0;j<cnt;j++){ incBits[eu[ord[j]]] |= 1ULL<<j; incBits[ev[ord[j]]] |= 1ULL<<j; }

    memset(ssum,0,sizeof ssum); used=0; nodes=0; aborted=0;
    int r;
    if (docount){ long long c = cnt_rec(0); labelings += c; r = (c>0); }
    else r = rec(0);
    if (nodes > worst_nodes){ worst_nodes = nodes; worst_mask = mask; }
    if (aborted) undec++;
    else if (r) anti++;
  }
  printf("n=%d shard=%d/%d connected=%lld antimagic=%lld undecided=%lld labelings=%lld worst_nodes=%lld worst_mask=%lld\n",
         n, shard, nsh, conn, anti, undec, labelings, worst_nodes, worst_mask);
  return 0;
}
