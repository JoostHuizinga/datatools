/*
 * common_compare.hpp
 *
 *  Created on: Aug 13, 2014
 *      Author: joost
 */

#ifndef COMMON_COMPARE_HPP_
#define COMMON_COMPARE_HPP_

// Standard includes
#include <cstddef>       // size_t
#include <algorithm>     // std::sort
#include <vector>        // std::vector
#include <numeric>       // std::accumulate
#include <cmath>         // std::fabs

// Sferes includes
#include <sferes/dbg/dbg.hpp>
#include <sferes/stc.hpp>

namespace sferes
{
namespace compare
{

template<typename ContainerType, typename ComparatorType>
inline void sort(ContainerType& container, ComparatorType comparator){
	return std::sort(container.begin(), container.end(), comparator);
}

template<typename ContainerType>
inline void sort(ContainerType& container){
	return std::sort(container.begin(), container.end());
}

template<typename ElementType>
inline ElementType& min(std::vector<ElementType>& container){
	return *std::min_element(container.begin(), container.end());
}

template<typename ElementType, typename ComparatorType>
inline ElementType& min(std::vector<ElementType>& container, ComparatorType comparator){
	return *std::min_element(container.begin(), container.end(), comparator);
}

template<typename ElementType>
inline ElementType& max(std::vector<ElementType>& container){
	return *std::max_element(container.begin(), container.end());
}

template<typename ElementType, typename ComparatorType>
inline ElementType& max(std::vector<ElementType>& container, ComparatorType comparator){
	return *std::max_element(container.begin(), container.end(), comparator);
}

template<typename ElementType, typename ComparatorType>
inline const ElementType& max(const std::vector<ElementType>& container, ComparatorType comparator){
    return *std::max_element(container.begin(), container.end(), comparator);
}

template<typename ContainerType>
inline float average(ContainerType& container){
	return std::accumulate(container.begin(), container.end(), 0.0f) / container.size();
}

inline double average(const std::vector<double>& container){
	return std::accumulate(container.begin(), container.end(), 0.0) / container.size();
}

template<typename ContainerType>
inline void merge(ContainerType& c1, const ContainerType& c2){
	c1.insert(c1.end(), c2.begin(), c2.end());
}

inline int mod(int a, int b){
	return (a%b+b)%b;
}

inline float l1diff(const std::vector<float>& container1, const std::vector<float>& container2){
    dbg::assertion(DBG_ASSERTION(container1.size() == container2.size()));
    dbg::out(dbg::info, "l1diff") << "container1 size: " << container1.size() << " container2 size: " <<  container2.size() << std::endl;
    float result=0;
    std::vector<float>::const_iterator it1;
    std::vector<float>::const_iterator it2;
    for(it1 = container1.begin(), it2 = container2.begin(); it1 != container1.end(); ++it1, ++it2){
        result += std::fabs(float(*it1) - float(*it2));
        dbg::out(dbg::info, "l1diff") << "container1 value: " << float(*it1) << " container2 value: " <<  float(*it2) << " absdiff: " << std::fabs(float(*it1) - float(*it2)) << std::endl;
    }
    return result / float(container1.size());
}

template<typename ElementType>
ElementType multiply (ElementType x, ElementType y) {return x*y;}

template<typename ElementType>
inline ElementType mult(const std::vector<ElementType>& container){
    return std::accumulate(container.begin(), container.end(), 1.0, multiply<ElementType>);
}

template<typename ElementType>
void gatherBest(const std::vector<ElementType>& container, std::vector<ElementType>& result){
    if(result.size() != 0){
        std::cerr << "Result vector is not empty!" << std::endl;
    }
    for (size_t i = 0; i < container.size() && container[i]->fit().value() == container[0]->fit().value(); ++i){
        result.push_back(container[i]);
    }
    if(result.size() == 0){
        std::cerr << "Result vector is empty after gathering!" << std::endl;
        std::cerr << "Container size: " << container.size() << std::endl;
        if(container.size() != 0){
            std::cerr << "Container[0] fit: " << container[0]->fit().value() << std::endl;
        }   
    }
}

struct stats{
    float min;
    float max;
    float avg;
    float median;
    float first_q;
    float third_q;
};

inline float getPercentile(const std::vector<float>& container, float percentile){
    float indexf = (float(container.size()) - 1.0) * percentile;
    float indexl = std::floor(indexf);
    float indexh = std::ceil(indexf);
    float remain = indexf - indexl;
    return container[(std::size_t)indexl]*remain + container[(std::size_t)indexh]*(1-remain);
}

inline stats sortAndCalcStats(std::vector<float>& container){
    stats result;
    sort(container);
    result.min = container[0];
    result.max = container[container.size()-1];
    result.avg = average(container);
    result.median = getPercentile(container, 0.5);
    result.first_q = getPercentile(container, 0.25);
    result.third_q = getPercentile(container, 0.75);
    return result;
}


template<typename Exact = stc::Itself>
class compare: public stc::Any<Exact>{
public:
    enum sort_order{
        order_ascending,
        order_descending
    };

	compare(sort_order order = order_ascending): _smaller(true), _greater(false), _equal(false){
	    switch(order){
	    case order_ascending:
	        ascending();
	        break;
	    case order_descending:
	        descending();
	        break;
	    default:
	        dbg::sentinel(DBG_HERE);
	    }
	}

	inline Exact& ascending(){
		smallerThan();
		return *stc::exact(this);
	}

	inline Exact& descending(){
		greaterThan();
		return *stc::exact(this);
	}

	inline void greaterThan(){
		_smaller = false;
		_greater = true;
		_equal = false;
	}

	inline void smallerThan(){
		_smaller = true;
		_greater = false;
		_equal = false;
	}

protected:
	bool _smaller, _greater, _equal;
};

// lexical order

/**
 * First sort on fit().value(), then sort on objectives in lexical order.
 */
struct val_objs
{
	template<typename I>
	bool operator() (const boost::shared_ptr<I> i1, const boost::shared_ptr<I> i2) const
	{
		assert(i1->fit().objs().size() == i2->fit().objs().size());
		assert(i1->fit().objs().size());
		assert(i2->fit().objs().size());
		// first, compare the main objective
		if (i1->fit().value() > i2->fit().value())
			return true;
		if (i1->fit().value() < i2->fit().value())
			return false;
		// equality : compare other objectives
		for (size_t i = 0; i < i1->fit().objs().size(); ++i){
			if (i1->fit().obj(i) > i2->fit().obj(i)){
				return true;
			}else if (i1->fit().obj(i) < i2->fit().obj(i)){
				return false;
			}
		}
		return false;
	}
};

/**
 * First sort on pareto dominance, then sort on objectives in lexical order.
 *
 * Implements the 'smaller than' (<) operator, which is assumed by std::max_element and std::min_element
 */
class pareto_objs: public compare<pareto_objs>
{
public:
    pareto_objs(){
    }

    template<typename I>
    bool operator() (const boost::shared_ptr<I> i1, const boost::shared_ptr<I> i2) const
    {
        dbg::assertion(DBG_ASSERTION(i1->fit().objs().size() == i2->fit().objs().size()));
        dbg::assertion(DBG_ASSERTION(i1->fit().objs().size()));
        dbg::assertion(DBG_ASSERTION(i2->fit().objs().size()));



        //Does i2 dominate i1
        bool i2_dominates_i1 = true;
        bool i1_dominates_i2 = true;
        bool i1_equal_to_i2 = true;

        for (size_t i = 0; i < i1->fit().objs().size(); ++i){
            dbg::out(dbg::info, "compare") << "obj " << i << ": " << i1->fit().obj(i) << " vs " << i2->fit().obj(i) << std::endl;
            if(i1->fit().obj(i) < i2->fit().obj(i)){
                i1_equal_to_i2 = false;
                i1_dominates_i2 = false;
            } else if (i1->fit().obj(i) > i2->fit().obj(i)){
                i1_equal_to_i2 = false;
                i2_dominates_i1 = false;
            }
        }
        if(i1_equal_to_i2){
            dbg::out(dbg::info, "compare") << "equal" << std::endl;
            return this->_equal;
        }
        if(i1_dominates_i2){
            dbg::out(dbg::info, "compare") << "greater" << std::endl;
            return this->_greater;
        }
        if(i2_dominates_i1){
            dbg::out(dbg::info, "compare") << "smaller" << std::endl;
            return this->_smaller;
        }


        // equality : compare other objectives
        for (size_t i = 0; i < i1->fit().objs().size(); ++i){
            if (i1->fit().obj(i) < i2->fit().obj(i)){
                dbg::out(dbg::info, "compare") << "smaller" << std::endl;
                return this->_smaller;
            }else if (i1->fit().obj(i) > i2->fit().obj(i)){
                dbg::out(dbg::info, "compare") << "greater" << std::endl;
                return this->_greater;
            }
        }
        dbg::out(dbg::info, "compare") << "equal" << std::endl;
        return this->_equal;
    }
};


/**
 * Compare on performance (fit().value()) first, modularity second (fit().mod())
 * Implements the 'smaller than' (<) operator, which is assumed by std::max_element and std::min_element
 */
class performance: public compare<performance>
{
public:
    template<typename I>
    bool operator() (const boost::shared_ptr<I> i1, const boost::shared_ptr<I> i2) const
    {
        // first, compare the main objective
        if (i1->fit().value() < i2->fit().value()) return this->_smaller;
        if (i1->fit().value() > i2->fit().value()) return this->_greater;
        return this->_equal;
    }
};


/**
 * Compare on performance (fit().value()) first, modularity second (fit().mod())
 * Implements the 'smaller than' (<) operator, which is assumed by std::max_element and std::min_element
 */
class performance_mod: public compare<performance_mod>
{
public:
	template<typename I>
	bool operator() (const boost::shared_ptr<I> i1, const boost::shared_ptr<I> i2) const
	{
		// first, compare the main objective
		if (i1->fit().value() < i2->fit().value()) return this->_smaller;
		if (i1->fit().value() > i2->fit().value()) return this->_greater;
		if (i1->fit().mod() < i2->fit().mod()) return this->_smaller;
		if (i1->fit().mod() > i2->fit().mod()) return this->_greater;
		return this->_equal;
	}
};

/**
 * Compare on performance (fit().value()) first, modularity second (fit().mod(), and CPPN modularity third (fit.cppn_mod())
 * Implements the 'smaller than' (<) operator, which is assumed by std::max_element and std::min_element
 */
class performance_mod_cppnmod: public compare<performance_mod>
{
public:
    template<typename I>
    bool operator() (const boost::shared_ptr<I> i1, const boost::shared_ptr<I> i2) const
    {
        // first, compare the main objective
        if (i1->fit().value() < i2->fit().value()) return this->_smaller;
        if (i1->fit().value() > i2->fit().value()) return this->_greater;
        if (i1->fit().mod() < i2->fit().mod()) return this->_smaller;
        if (i1->fit().mod() > i2->fit().mod()) return this->_greater;
        if (i1->fit().cppn_mod() < i2->fit().cppn_mod()) return this->_smaller;
        if (i1->fit().cppn_mod() > i2->fit().cppn_mod()) return this->_greater;
        return this->_equal;
    }
};


// use fitness::values()
struct values
{
	values(size_t k) : _k(k) {
	}
	template<typename I>
	bool operator() (const boost::shared_ptr<I>&i1,
			const boost::shared_ptr<I>&i2) const
	{
		return i1->fit().values(_k) > i2->fit().values(_k);
	}
	size_t _k;
};


}
}


#endif /* COMMON_COMPARE_HPP_ */
