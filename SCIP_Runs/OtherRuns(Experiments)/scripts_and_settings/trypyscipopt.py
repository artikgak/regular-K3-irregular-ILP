from pyscipopt import Model

model = Model("example")

x = model.addVar("x", vtype="B")
y = model.addVar("y", vtype="B")

model.addCons(3*x + 2*y <= 100)

model.setObjective(x + y, "maximize")

model.optimize()

print(model.getVal(x), model.getVal(y))