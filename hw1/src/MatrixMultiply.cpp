#include "MatrixMultiply.hpp"

#include <exception>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <numeric>

scottgs::MatrixMultiply::MatrixMultiply() 
{
	;
}

scottgs::MatrixMultiply::~MatrixMultiply()
{
	;
}


scottgs::FloatMatrix scottgs::MatrixMultiply::operator()(const scottgs::FloatMatrix& lhs, const scottgs::FloatMatrix& rhs) const
{
	// Verify acceptable dimensions
	if (lhs.size2() != rhs.size1())
		throw std::logic_error("matrix incompatible lhs.size2() != rhs.size1()");

	scottgs::FloatMatrix result(lhs.size1(),rhs.size2());


	// YOUR ALGORIHM WITH COMMENTS GOES HERE:
	//Getting rid of read function calls
	const float result_size_one = result.size1();
	const float result_size_two = result.size2();
	const float lhs_size_two = lhs.size2();

	//Basic Triple For-Loop Solution O(n^3)
    for (unsigned int i = 0; i < result_size_one; ++i){
        for (unsigned int j = 0; j < result_size_two; ++j){

			float temp = 0.0; //better to use temp than to add to double each time, less function calls...I think
            for (unsigned int inner = 0; inner < lhs_size_two; ++inner){
                temp += lhs(i,inner) * rhs(inner,j);
            }
			result(i,j) = temp;
        }
    }
	return result;
}

scottgs::FloatMatrix scottgs::MatrixMultiply::multiply(const scottgs::FloatMatrix& lhs, const scottgs::FloatMatrix& rhs) const
{
	// Verify acceptable dimensions
	if (lhs.size2() != rhs.size1())
		throw std::logic_error("matrix incompatible lhs.size2() != rhs.size1()");

	return boost::numeric::ublas::prod(lhs,rhs);
}

