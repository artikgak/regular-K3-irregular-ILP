# SCIP_irreg

Program to generate lp files to test if regular K_3-irregular graphs exits for given regulairy and order

## Some commands to run script [todo, edit them]

"C:\Program Files\SCIPOptSuite 10.0.1\bin\fscip.exe" default.prm reg_irreg_N14_R7.lp -sth 12 -fsol solution.sol > runlog12th.txt

"C:\Program Files\SCIPOptSuite 10.0.1\bin\fscip.exe" default.prm reg_irreg_N24_R9.lp -sth 4 -fsol solution.sol -isol solStart_N24_R92.mst

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N24_R9.lp optimize"

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N24_R9.lp read solStart_N24_R9.mst optimize"

"C:\Program Files\SCIPOptSuite 10.0.1\bin\fscip.exe" default.prm reg_irreg_N22_R8.lp -sth 12

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N18_R8_test.lp set emphasis feasibility set heuristics emphasis aggressive optimize" // not good

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N18_R8_test.lp set emphasis feasibility optimize" // worse

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N18_R8_test.lp set heuristics emphasis aggressive optimize" /// better

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N22_R_spec8_t.lp set heuristics emphasis off set presolving emphasis aggressive set separating emphasis aggressive set nodeselection restartdfs stdpriority 100000 set conflict useinflp  b optimize display solution"

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N14_R8.lp set heuristics emphasis off set presolving emphasis aggressive set separating emphasis aggressive set nodeselection restartdfs stdpriority 100000 set conflict useinflp  b optimize display solution

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N14_R_spec8.lp set heuristics emphasis off set presolving emphasis aggressive set separating emphasis aggressive set nodeselection restartdfs stdpriority 100000 set conflict useinflp  b optimize display solution

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N15_R8.lp set heuristics emphasis off set presolving emphasis aggressive set separating emphasis aggressive set nodeselection restartdfs stdpriority 100000 set conflict useinflp  b optimize display solution

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N15_R_spec8.lp set heuristics emphasis off set presolving emphasis aggressive set separating emphasis aggressive set nodeselection restartdfs stdpriority 100000 set conflict useinflp  b optimize display solution

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N16_R_spec8.lp set heuristics emphasis off set presolving emphasis aggressive set separating emphasis aggressive set nodeselection restartdfs stdpriority 100000 set conflict useinflp  b optimize display solution

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N20_R_spec8.lp set heuristics emphasis off set presolving emphasis aggressive set separating emphasis aggressive set nodeselection restartdfs stdpriority 100000 set conflict useinflp  b optimize display solution

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N24_R_spec9.lp set heuristics completesol maxunknownrate 0.99  read presolve9r_split.mst optimize display solution quit" > sol24_9.log

"C:\Program Files\SCIPOptSuite 10.0.1\bin\scip.exe" -c "read reg_irreg_N24_R_spec9.lp set heuristics completesol maxunknownrate 0.99  read presolve9r_split.mst optimize write solution solution.txt"
