package com.poorcraft.common.world.gen.noise;

/**
 * OpenSimplex2S noise implementation.
 * Smooth variant optimized for terrain generation.
 * Based on KdotJPG's OpenSimplex2 algorithm (CC0 license).
 */
public class OpenSimplex2S {
    
    private static final long PRIME_X = 0x5205402B9270C86FL;
    private static final long PRIME_Y = 0x598CD327003817B5L;
    private static final long PRIME_Z = 0x5BCC226E9FA0BACBL;
    private static final double ROOT2OVER2 = 0.7071067811865476;
    private static final double ROOT3OVER3 = 0.577350269189626;
    
    private static final int PSIZE = 2048;
    private static final int PMASK = 2047;
    
    private final short[] perm;
    private final Grad2[] permGrad2;
    private final Grad3[] permGrad3;
    
    public OpenSimplex2S(long seed) {
        perm = new short[PSIZE];
        permGrad2 = new Grad2[PSIZE];
        permGrad3 = new Grad3[PSIZE];
        
        short[] source = new short[PSIZE];
        for (short i = 0; i < PSIZE; i++) {
            source[i] = i;
        }
        
        for (int i = PSIZE - 1; i >= 0; i--) {
            seed = seed * 6364136223846793005L + 1442695040888963407L;
            int r = (int)((seed + 31) % (i + 1));
            if (r < 0) r += (i + 1);
            perm[i] = source[r];
            permGrad2[i] = GRADIENTS_2D[perm[i]];
            permGrad3[i] = GRADIENTS_3D[perm[i]];
            source[r] = source[i];
        }
    }
    
    /**
     * 2D OpenSimplex2S noise.
     */
    public double noise2(long seed, double x, double y) {
        double s = (x + y) * 0.366025403784439;
        double xs = x + s, ys = y + s;
        
        return noise2_Base(seed, xs, ys);
    }
    
    /**
     * 3D OpenSimplex2S noise, ImproveXZ variant for terrain (Y is vertical).
     */
    public double noise3_ImproveXZ(long seed, double x, double y, double z) {
        double xy = x + y;
        double s2 = xy * -0.211324865405187;
        double zz = z * 0.577350269189626;
        double xr = x + s2 - zz;
        double yr = y + s2 - zz;
        double zr = xy * 0.577350269189626 + zz;
        
        return noise3_Base(seed, xr, yr, zr);
    }
    
    private double noise2_Base(long seed, double xs, double ys) {
        int xsb = fastFloor(xs);
        int ysb = fastFloor(ys);
        double xsi = xs - xsb;
        double ysi = ys - ysb;
        
        double ssi = (xsi + ysi) * -0.211324865405187;
        double xi = xsi + ssi;
        double yi = ysi + ssi;
        
        double value = 0;
        
        // Corner 1
        double dx1 = xi;
        double dy1 = yi;
        double attn1 = 2.0 / 3.0 - dx1 * dx1 - dy1 * dy1;
        if (attn1 > 0) {
            int pxm = (xsb * PRIME_X) & PMASK;
            int pym = (ysb * PRIME_Y) & PMASK;
            Grad2 grad = permGrad2[perm[pxm] ^ pym];
            double extrapolation = grad.dx * dx1 + grad.dy * dy1;
            attn1 *= attn1;
            value += attn1 * attn1 * extrapolation;
        }
        
        // Corner 2
        double dx2 = xi - 1;
        double dy2 = yi - 0;
        double attn2 = 2.0 / 3.0 - dx2 * dx2 - dy2 * dy2;
        if (attn2 > 0) {
            int pxm = ((xsb + 1) * PRIME_X) & PMASK;
            int pym = (ysb * PRIME_Y) & PMASK;
            Grad2 grad = permGrad2[perm[pxm] ^ pym];
            double extrapolation = grad.dx * dx2 + grad.dy * dy2;
            attn2 *= attn2;
            value += attn2 * attn2 * extrapolation;
        }
        
        // Corner 3
        double dx3 = xi - 0;
        double dy3 = yi - 1;
        double attn3 = 2.0 / 3.0 - dx3 * dx3 - dy3 * dy3;
        if (attn3 > 0) {
            int pxm = (xsb * PRIME_X) & PMASK;
            int pym = ((ysb + 1) * PRIME_Y) & PMASK;
            Grad2 grad = permGrad2[perm[pxm] ^ pym];
            double extrapolation = grad.dx * dx3 + grad.dy * dy3;
            attn3 *= attn3;
            value += attn3 * attn3 * extrapolation;
        }
        
        return value;
    }
    
    private double noise3_Base(long seed, double xr, double yr, double zr) {
        int xrb = fastFloor(xr);
        int yrb = fastFloor(yr);
        int zrb = fastFloor(zr);
        double xri = xr - xrb;
        double yri = yr - yrb;
        double zri = zr - zrb;
        
        double value = 0;
        
        // Simplified 3D - evaluate 4 corners
        for (int i = 0; i < 4; i++) {
            int xi = (i & 1);
            int yi = ((i >> 1) & 1);
            int zi = ((i >> 2) & 1);
            
            double dx = xri - xi;
            double dy = yri - yi;
            double dz = zri - zi;
            
            double attn = 0.6 - dx * dx - dy * dy - dz * dz;
            if (attn > 0) {
                int pxm = ((xrb + xi) * PRIME_X) & PMASK;
                int pym = ((yrb + yi) * PRIME_Y) & PMASK;
                int pzm = ((zrb + zi) * PRIME_Z) & PMASK;
                Grad3 grad = permGrad3[perm[perm[pxm] ^ pym] ^ pzm];
                double extrapolation = grad.dx * dx + grad.dy * dy + grad.dz * dz;
                attn *= attn;
                value += attn * attn * extrapolation;
            }
        }
        
        return value;
    }
    
    private static int fastFloor(double x) {
        int xi = (int) x;
        return x < xi ? xi - 1 : xi;
    }
    
    private static class Grad2 {
        double dx, dy;
        public Grad2(double dx, double dy) {
            this.dx = dx;
            this.dy = dy;
        }
    }
    
    private static class Grad3 {
        double dx, dy, dz;
        public Grad3(double dx, double dy, double dz) {
            this.dx = dx;
            this.dy = dy;
            this.dz = dz;
        }
    }
    
    private static final Grad2[] GRADIENTS_2D;
    private static final Grad3[] GRADIENTS_3D;
    
    static {
        GRADIENTS_2D = new Grad2[PSIZE];
        Grad2[] grad2 = {
            new Grad2( 0.130526192220052,  0.99144486137381),
            new Grad2( 0.38268343236509,   0.923879532511287),
            new Grad2( 0.608761429008721,  0.793353340291235),
            new Grad2( 0.793353340291235,  0.608761429008721),
            new Grad2( 0.923879532511287,  0.38268343236509),
            new Grad2( 0.99144486137381,   0.130526192220051),
            new Grad2( 0.99144486137381,  -0.130526192220051),
            new Grad2( 0.923879532511287, -0.38268343236509),
            new Grad2( 0.793353340291235, -0.60876142900872),
            new Grad2( 0.608761429008721, -0.793353340291235),
            new Grad2( 0.38268343236509,  -0.923879532511287),
            new Grad2( 0.130526192220052, -0.99144486137381),
            new Grad2(-0.130526192220052, -0.99144486137381),
            new Grad2(-0.38268343236509,  -0.923879532511287),
            new Grad2(-0.608761429008721, -0.793353340291235),
            new Grad2(-0.793353340291235, -0.608761429008721),
            new Grad2(-0.923879532511287, -0.38268343236509),
            new Grad2(-0.99144486137381,  -0.130526192220052),
            new Grad2(-0.99144486137381,   0.130526192220051),
            new Grad2(-0.923879532511287,  0.38268343236509),
            new Grad2(-0.793353340291235,  0.608761429008721),
            new Grad2(-0.608761429008721,  0.793353340291235),
            new Grad2(-0.38268343236509,   0.923879532511287),
            new Grad2(-0.130526192220052,  0.99144486137381)
        };
        for (int i = 0; i < PSIZE; i++) {
            GRADIENTS_2D[i] = grad2[i % grad2.length];
        }
        
        GRADIENTS_3D = new Grad3[PSIZE];
        Grad3[] grad3 = {
            new Grad3(-2.22474487139,      -2.22474487139,      -1.0),
            new Grad3(-2.22474487139,      -2.22474487139,       1.0),
            new Grad3(-3.0862664687972017, -1.1721513422464978,  0.0),
            new Grad3(-1.1721513422464978, -3.0862664687972017,  0.0),
            new Grad3(-2.22474487139,      -1.0,                -2.22474487139),
            new Grad3(-2.22474487139,       1.0,                -2.22474487139),
            new Grad3(-1.1721513422464978,  0.0,                -3.0862664687972017),
            new Grad3(-3.0862664687972017,  0.0,                -1.1721513422464978),
            new Grad3(-2.22474487139,      -1.0,                 2.22474487139),
            new Grad3(-2.22474487139,       1.0,                 2.22474487139),
            new Grad3(-3.0862664687972017,  0.0,                 1.1721513422464978),
            new Grad3(-1.1721513422464978,  0.0,                 3.0862664687972017),
            new Grad3(-2.22474487139,       2.22474487139,      -1.0),
            new Grad3(-2.22474487139,       2.22474487139,       1.0),
            new Grad3(-1.1721513422464978,  3.0862664687972017,  0.0),
            new Grad3(-3.0862664687972017,  1.1721513422464978,  0.0)
        };
        for (int i = 0; i < PSIZE; i++) {
            GRADIENTS_3D[i] = grad3[i % grad3.length];
        }
    }
}
