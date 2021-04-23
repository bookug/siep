# siep

implementation of state-of-the-art subgraph matching algorithms (including BoostISO)

---

# Papers

Li Zeng, Yan Jiang, Weixin Lu, Lei Zou. [Deep Analysis on Subgraph Isomorphism](https://arxiv.org/abs/2012.06802). arXiv 2020.

---

# Algorithms

Seven subgraph matching algorithms: QuickSI, GraphQL, TurboISO, CFL-Match, VF3, CECI, DAF.

Each algorithm also has a boosted version, which is enhanced by techniques of BoostISO.

---

# Contributors

Li Zeng @bookug

1. Implement TurboISO, CECI, DAF.
2. Fix and optimize QuickSI and GraphQL.
3. Implement all boosted versions except for CFL-Match and VF3.


Yan Jiang @jy5275

1. Implement CFL-Match and VF3, as well as their boosted versions.
2. Performance tools, debugging, testing, running scripts, etc.


Weixin Lu @aceansgar

1. Implement QuickSI and GraphQL.
2. Tools of graph benchmark (e.g., query generation, power-law graph detection, etc).
