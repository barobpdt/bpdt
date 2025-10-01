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


## use numpy

import numpy as np

def interpolant(t):
    return t*t*t*(t*(t*6 - 15) + 10)

def generate_perlin_noise_2d(
        shape, res, tileable=(False, False), interpolant=interpolant
):
    """Generate a 2D numpy array of perlin noise.

    np.random.seed(0)
    noise = generate_perlin_noise_2d((256, 256), (8, 8))
    plt.imshow(noise, cmap='gray', interpolation='lanczos')
    plt.colorbar()
    Args:
        shape: The shape of the generated array (tuple of two ints).
            This must be a multple of res.
        res: The number of periods of noise to generate along each
            axis (tuple of two ints). Note shape must be a multiple of
            res.
        tileable: If the noise should be tileable along each axis
            (tuple of two bools). Defaults to (False, False).
        interpolant: The interpolation function, defaults to
            t*t*t*(t*(t*6 - 15) + 10).

    Returns:
        A numpy array of shape shape with the generated noise.

    Raises:
        ValueError: If shape is not a multiple of res.
    """
    delta = (res[0] / shape[0], res[1] / shape[1])
    d = (shape[0] // res[0], shape[1] // res[1])
    grid = np.mgrid[0:res[0]:delta[0], 0:res[1]:delta[1]]\
             .transpose(1, 2, 0) % 1
    # Gradients
    angles = 2*np.pi*np.random.rand(res[0]+1, res[1]+1)
    gradients = np.dstack((np.cos(angles), np.sin(angles)))
    if tileable[0]:
        gradients[-1,:] = gradients[0,:]
    if tileable[1]:
        gradients[:,-1] = gradients[:,0]
    gradients = gradients.repeat(d[0], 0).repeat(d[1], 1)
    g00 = gradients[    :-d[0],    :-d[1]]
    g10 = gradients[d[0]:     ,    :-d[1]]
    g01 = gradients[    :-d[0],d[1]:     ]
    g11 = gradients[d[0]:     ,d[1]:     ]
    # Ramps
    n00 = np.sum(np.dstack((grid[:,:,0]  , grid[:,:,1]  )) * g00, 2)
    n10 = np.sum(np.dstack((grid[:,:,0]-1, grid[:,:,1]  )) * g10, 2)
    n01 = np.sum(np.dstack((grid[:,:,0]  , grid[:,:,1]-1)) * g01, 2)
    n11 = np.sum(np.dstack((grid[:,:,0]-1, grid[:,:,1]-1)) * g11, 2)
    # Interpolation
    t = interpolant(grid)
    n0 = n00*(1-t[:,:,0]) + t[:,:,0]*n10
    n1 = n01*(1-t[:,:,0]) + t[:,:,0]*n11
    return np.sqrt(2)*((1-t[:,:,1])*n0 + t[:,:,1]*n1)


def generate_fractal_noise_2d(
        shape, res, octaves=1, persistence=0.5,
        lacunarity=2, tileable=(False, False),
        interpolant=interpolant
):
    """Generate a 2D numpy array of fractal noise.

    Args:
        shape: The shape of the generated array (tuple of two ints).
            This must be a multiple of lacunarity**(octaves-1)*res.
        res: The number of periods of noise to generate along each
            axis (tuple of two ints). Note shape must be a multiple of
            (lacunarity**(octaves-1)*res).
        octaves: The number of octaves in the noise. Defaults to 1.
        persistence: The scaling factor between two octaves.
        lacunarity: The frequency factor between two octaves.
        tileable: If the noise should be tileable along each axis
            (tuple of two bools). Defaults to (False, False).
        interpolant: The, interpolation function, defaults to
            t*t*t*(t*(t*6 - 15) + 10).

    Returns:
        A numpy array of fractal noise and of shape shape generated by
        combining several octaves of perlin noise.

    Raises:
        ValueError: If shape is not a multiple of
            (lacunarity**(octaves-1)*res).
    """
    noise = np.zeros(shape)
    frequency = 1
    amplitude = 1
    for _ in range(octaves):
        noise += amplitude * generate_perlin_noise_2d(
            shape, (frequency*res[0], frequency*res[1]), tileable, interpolant
        )
        frequency *= lacunarity
        amplitude *= persistence
    return noise    