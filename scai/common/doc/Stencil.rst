.. _Stencil:

Stencil
=======

Stencil is a class to define a linear update of an element in a grid
by the values of some neighbors. This linear mapping is the same for
all grid values, except at the boundaries where some other rules apply.

Therefore a stencil stores for each relevant neighbor (stencil point)
the relative coordinates of the neighbor and the corresponding value.

.. code-block:: c++

    double v_left  = 0.2;
    double v_right = 0.2;
    double v_lower = 0.2;
    double v_upper = 0.2;
    double v_self  = 0.2;

    Stencil2D<double> stencil;

    stencil.addPoint( 0, -1, v_left );
    stencil.addPoint( 0, 1, v_right );
    stencil.addPoint( -1, 0, v_lower );
    stencil.addPoint(  1, 0, v_upper );
    stencil.addPoint(  0, 0, v_self );

If this stencil is applied to a two-dimensional grid the values 
are updated as follows:

.. code-block:: c++

    out(i,j) = v_self * in(i,j) + v_left * in(i,j-1) + v_right * in(i,j+1) 
                                + v_lower * in(i-1,j) + v_upper * in(i+1,j) 

For each dimension an own stencil class is provided where all these 
classes derive from the common base class ``Stencil``.


A stencil and a grid togehter are used in LAMA to define a stencil matrix
that specifies the linear mapping from the input values to the output values
for all grid points.

Some very common stencils can be constructed as follows:

.. code-block:: c++

     Stencil1D stencil1D3P( 3 );
     Stencil2D stencil2D5P( 5 );
     Stencil2D stencil2D9P( 9 );
     Stencil3D stencil3D7P( 7 );
     Stencil3D stencil3D27P( 27 );