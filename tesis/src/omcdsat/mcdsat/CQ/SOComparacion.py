class SOComparacion:
    def __init__(self, u, v, theta):
        self.u = u
        self.v = v
        self.theta = theta

    def __str__(self):
        u1, val = self.u
        v1, val = self.v
        return u1 + self.theta + v1

    def map_variables(self, mapping):
        u1, val1 = self.u
        v1, val2 = self.v
        if val1 == 0 and mapping.has_key(u1):
            u_ret = mapping[u1], 0
        else:
            u_ret = u1, val1
        if val2 == 0 and mapping.has_key(v1):
            v_ret = mapping[v1], 0
        else:
            v_ret = v1,val2
        return SOComparacion(u_ret, v_ret, self.theta)
        
            
            
