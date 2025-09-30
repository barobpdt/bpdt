import itertools
from functools import cache

#--- Mathematics ---
import numpy as np

#****************************************************************************************************
#                                          Noise Generation                                          
#****************************************************************************************************

class NoiseGenerator():

    #================================================================================
    # Initialization
    #================================================================================

    def __init__(self,seed=0,nr_dimensions=2):
        self.SEED          = int(seed)
        self.NR_DIMENSIONS = int(nr_dimensions)

        self.fade_f = smootherstep

        self.compute_constants()

    def compute_constants(self):
        self.PERMUTATION_TABLE = get_permutation_table(self.SEED)
        self.CORNERS           = get_corners  (self.NR_DIMENSIONS)
        self.GRADIENTS         = get_gradients(self.NR_DIMENSIONS)

        # Extend memory, to avoid '%' operation when retrieving gradient indices!
        self.NR_GRADIENTS       = self.GRADIENTS.shape[0]
        GRADIENT_MULTIPLIER     = int(np.ceil(self.PERMUTATION_TABLE.shape[0]/self.NR_GRADIENTS))
        self.GRADIENTS_EXTENDED = np.vstack([self.GRADIENTS]*GRADIENT_MULTIPLIER)

    #================================================================================
    # Generation
    #================================================================================
    
    def fractal_noise(self,pos,octaves=8):
        noise = np.zeros(pos.shape[:-1])
        for i in range(octaves):
            freq = 2**i
            amp  = 1/freq
            noise+= self.perlin_noise(pos*freq) * amp
        return noise

    def perlin_noise(self,pos):
        pos_i     = pos.astype(int)                                               # Grid coordinates
        pos_f     = pos - pos_i                                                   # Local fractional coordinates
        gradients = {tuple(c):self.get_gradients (pos_i+c) for c in self.CORNERS} # Grid gradients               # ToDo: Remove duplicate computation!
        n         = [self.dot(gradients[tuple(c)],pos_f-c) for c in self.CORNERS] # Noise components
        pos_ff    = self.fade_f(pos_f)                                            # Fade positions
        for i in range(self.NR_DIMENSIONS):                                       # Interpolate noise
            n     = [lerp(n1,n2, pos_ff[self.filter_axis(i)]) for n1,n2 in zip(n[:len(n)//2],n[len(n)//2:])]
        return n[0]
    
    #================================================================================
    # Support Functions
    #================================================================================
    
    def get_pos_grid(self,dim=512):
        return np.moveaxis(np.mgrid[[slice(0,dim)]*self.NR_DIMENSIONS],0,self.NR_DIMENSIONS)/dim

    def get_gradients(self,pos):
        return self.GRADIENTS_EXTENDED[self.get_gradients_idx(pos)]

    def get_gradients_idx(self,pos):
        gradient_idx = pos[self.filter_axis(0)]
        for i in range(1,self.NR_DIMENSIONS):
            gradient_idx = self.PERMUTATION_TABLE[gradient_idx+pos[self.filter_axis(i)]]
        return gradient_idx

    def dot(self,a,b):
        return np.sum([a[self.filter_axis(i)]*b[self.filter_axis(i)] for i in range(self.NR_DIMENSIONS)],axis=0)

    def filter_axis(self,axis):
        SLICE_ALL = [slice(None)]*self.NR_DIMENSIONS
        return tuple(SLICE_ALL+[axis])

#================================================================================
# Support functions
#================================================================================

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Constants
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

@cache
def get_permutation_table(seed=0,N=512):
    np.random.seed(seed)
    p = np.arange(N//2, dtype=int)
    np.random.shuffle(p)
    p = np.stack([p]*2).flatten()
    return p

def _get_combinations(nr_dimensions,vs):
    return np.array(list(itertools.product(*zip(*[[v]*nr_dimensions for v in vs]))))

@cache
def get_corners(nr_dimensions):
    return _get_combinations(nr_dimensions,[0,1])

@cache
def get_gradients(nr_dimensions):
    return _get_combinations(nr_dimensions,[-1,+1])

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Transitions
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

def lerp(a, b, r):
    return a + r * (b - a)

def smootherstep(t):
    t2 = t**2 # Pre-compute square, for faster computation
    return t2*t * (6*t2 - 15*t + 10)

#****************************************************************************************************
#                                              Test Code                                             
#****************************************************************************************************

if __name__=="__main__":

    #--- Imports ---
    import matplotlib.pyplot as plt
    import time

    #--- Settings ---
    PROFILE       = False
    NR_DIMENSIONS = 3
    DIM           = 2**(9-NR_DIMENSIONS)

    #--- Computation ---
    t0 = time.time()

    ng  = NoiseGenerator(nr_dimensions=NR_DIMENSIONS)
    pos = ng.get_pos_grid(DIM)
    if PROFILE:
        import cProfile
        import pstats
        cProfile.run("ng.fractal_noise(pos)","pstats.ps")
        p = pstats.Stats("pstats.ps")
        p.sort_stats(pstats.SortKey.TIME).print_stats(10)
        quit()
    else:
        noise = ng.fractal_noise(pos)

    dt = time.time()-t0
    print(f"Noise generated in {dt:.1f} s")

    #--- Visualization ---
    def nd_slice(nd):
        return tuple([slice(None)]*nd+[0]*(NR_DIMENSIONS-nd))

    if NR_DIMENSIONS>=1:
        plt.figure("1D")
        plt.plot(np.arange(noise.shape[0]),noise[nd_slice(1)],color="gray")
    if NR_DIMENSIONS>=2:
        plt.figure("2D")
        plt.imshow(noise[nd_slice(2)],cmap="gray")
    if NR_DIMENSIONS>=3:
        n = noise[nd_slice(3)]
        n_norm = (n-np.min(n))/(np.max(n)-np.min(n))
        colors = np.zeros(n.shape+(4,))
        for i in range(4): colors[:,:,:,i] = n_norm
        ax = plt.figure("3D").add_subplot(projection="3d")
        ax.set(xlabel="x", ylabel="y", zlabel="z")
        ax.voxels(
            *np.indices(np.array(n.shape)+1), n,
            facecolors=colors,
            linewidth=0.5
        )
    plt.show()