/*
 * simpleArray.h
 *
 *  Created on: 5 Nov 2019
 *      Author: jkiesele
 */

#ifndef DJCDEV_DEEPJETCORE_COMPILED_INTERFACE_SIMPLEARRAY_H_
#define DJCDEV_DEEPJETCORE_COMPILED_INTERFACE_SIMPLEARRAY_H_

#include <vector>
#include <string>
#include <stdio.h>
#include "quicklzWrapper.h"
#include <cstring> //memcpy
#include "IO.h"
#include "version.h"

#include <iostream>

#define USEMOVE

namespace djc{


template<class T>
class simpleArray {
public:

    simpleArray();
    // row splits are indicated by a merged dimension with negative sign
    // always merge to previous dimension
    // e.g. A x B x C x D, where B is ragged would get shape
    // A x -A*B x C x D
    // ROW SPLITS START WITH 0 and end with the total number of elements along that dimension
    // therefore, the rosplits vector size is one more than the first dimension
    //
    // Only ONLY DIMENSION 1 AS RAGGED DIMENSION is supported, first dimension MUST NOT be ragged.
    //

    simpleArray(std::vector<int> shape,const std::vector<size_t>& rowsplits = {});
    simpleArray(FILE *& );
    ~simpleArray();

    simpleArray(const simpleArray<T>&);
    simpleArray<T>& operator=(const simpleArray<T>&);

#ifdef USEMOVE
    simpleArray(simpleArray<T> &&);
    simpleArray<T>& operator=(simpleArray<T> &&);
#endif
    void clear();

    //reshapes if possible, creates new else
    void setShape(std::vector<int> shape,const std::vector<size_t>& rowsplits = {});

    T * data() const {
        return data_;
    }

    T * data() {
        return data_;
    }


    const std::vector<int>& shape() const {
        return shape_;
    }

    const size_t& size() const {
        return size_;
    }

    bool isRagged()const{
        return rowsplits_.size()>0;
    }

    /*
     * returns the dimension of the first axis.
     * If second dimension is ragged, this will take it into
     * account.
     */
    size_t getFirstDimension()const;


    const std::vector<size_t>& rowsplits() const {
        return rowsplits_;
    }



    /////////// potentially dangerous operations for conversions, use with care ///////

    /*
     * Move data memory location to another object
     * This will reset the array. Read shapes, row splits etc. before
     * performing this operation!
     */
    T * disownData();

    /*
     * Object will not own the data. Merely useful for conversion
     * with immediate writing to file
     */
    void assignData(T *d){
        if(data_ && !assigned_)
            delete data_;
        data_=d;
        assigned_=true;
    }

    /*
     * Assigns a shape without checking it or creating a new data
     * array. Will recalculate total size
     */
    void assignShape(std::vector<int> s){
        shape_=s;
        size_ = sizeFromShape(s);
    }

    /*
     * Splits on first axis.
     * Returns the first part, leaves the second.
     * does memcopy for both pats now
     */
    simpleArray<T> split(size_t splitindex);

    /*
     * appends along first axis
     * Cann append to an empty array (same as copy)
     */
    void append(const simpleArray<T>& a);

    /* file IO here
     * format: non compressed header (already writing rowsplits!):
     * size, shape.size(), [shape], rowsplits.size(), [rowsplits], compr: [data]
     *
     */
    void addToFile(FILE *& ofile) const;

    void readFromFile(FILE *& ifile);


    size_t sizeAt(size_t i)const;
    // higher dim row splits size_t sizeAt(size_t i,size_t j)const;
    // higher dim row splits size_t sizeAt(size_t i,size_t j, size_t k)const;
    // higher dim row splits size_t sizeAt(size_t i,size_t j, size_t k, size_t l)const;
    // higher dim row splits size_t sizeAt(size_t i,size_t j, size_t k, size_t l, size_t m)const;

    /*
     * Does not work (yet) with ragged arrays!
     * Will just produce garbage!
     */

    T & at(size_t i);
    const T & at(size_t i)const;
    T & at(size_t i, size_t j);
    const T & at(size_t i, size_t j)const;
    T & at(size_t i, size_t j, size_t k);
    const T & at(size_t i, size_t j, size_t k)const;
    T & at(size_t i, size_t j, size_t k, size_t l);
    const T & at(size_t i, size_t j, size_t k, size_t l)const;
    T & at(size_t i, size_t j, size_t k, size_t l, size_t m);
    const T & at(size_t i, size_t j, size_t k, size_t l, size_t m)const;
    T & at(size_t i, size_t j, size_t k, size_t l, size_t m, size_t n);
    const T & at(size_t i, size_t j, size_t k, size_t l, size_t m, size_t n)const;

private:
    size_t flatindex(size_t i, size_t j)const;
    size_t flatindex(size_t i, size_t j, size_t k)const;
    size_t flatindex(size_t i, size_t j, size_t k, size_t l)const;
    size_t flatindex(size_t i, size_t j, size_t k, size_t l, size_t m)const;
    size_t flatindex(size_t i, size_t j, size_t k, size_t l, size_t m, size_t n)const;


    void copyFrom(const simpleArray<T>& a);
    void moveFrom(simpleArray<T> && a);
    int sizeFromShape(std::vector<int> shape) const;
    std::vector<int> shapeFromRowsplits()const; //split dim = 1!
    void checkShape(size_t ndims)const;
    void checkSize(size_t idx)const;
    void checkRaggedIndex(size_t i, size_t j)const;

    T * data_;
    std::vector<int> shape_;
    std::vector<size_t> rowsplits_;
    size_t size_;
    bool assigned_;
};

template<class T>
simpleArray<T>::simpleArray() :
        data_(0), size_(0),assigned_(false) {

}

template<class T>
simpleArray<T>::simpleArray(std::vector<int> shape,const std::vector<size_t>& rowsplits) :
        data_(0), size_(0),assigned_(false) {

    shape_ = shape;
    if(rowsplits.size()){
        if(rowsplits.size() != shape_.at(0)+1)
            throw std::runtime_error("simpleArray<T>::simpleArray: rowsplits.size() must equal shape[0] + 1");

        rowsplits_=rowsplits;
        shape_ = shapeFromRowsplits();
    }
    size_ = sizeFromShape(shape_);
    data_ = new T[size_];
}

template<class T>
simpleArray<T>::simpleArray(FILE *& ifile):simpleArray<T>(){
    readFromFile(ifile);
    assigned_=false;
}

template<class T>
simpleArray<T>::~simpleArray() {
    clear();
}

template<class T>
simpleArray<T>::simpleArray(const simpleArray<T>& a) :
        simpleArray<T>() {
    copyFrom(a);
}

template<class T>
simpleArray<T>& simpleArray<T>::operator=(const simpleArray<T>& a) {
    copyFrom(a);
    return *this;
}

#ifdef USEMOVE
template<class T>
simpleArray<T>::simpleArray(simpleArray<T> && a) :
        simpleArray<T>() {
    if (&a == this){
        return;}
    if (data_&& !assigned_)
        delete data_;
    data_ = a.data_;
    a.data_ = 0;
    assigned_ = a.assigned_;
    size_ = a.size_;
    a.size_ = 0;
    shape_ = std::move(a.shape_);
    a.shape_ = std::vector<int>();
    rowsplits_ = std::move(a.rowsplits_);
    a.rowsplits_= std::vector<size_t>();
    a.clear();
}


template<class T>
simpleArray<T>& simpleArray<T>::operator=(simpleArray<T> && a) {
    if (&a == this)
        return *this;
    if (data_ && !assigned_)
        delete data_;
    data_ = a.data_;
    a.data_ = 0;
    size_ = a.size_;
    assigned_ = a.assigned_;
    a.size_ = 0;
    shape_ = std::move(a.shape_);
    a.shape_ = std::vector<int>();
    rowsplits_ = std::move(a.rowsplits_);
    a.rowsplits_= std::vector<size_t>();
    return *this;
}
#endif

template<class T>
void simpleArray<T>::clear() {
    if (data_&& !assigned_)
        delete data_;
    data_ = 0;
    shape_.clear();
    rowsplits_.clear();
    size_ = 0;
}

template<class T>
void simpleArray<T>::setShape(std::vector<int> shape,const std::vector<size_t>& rowsplits) {
    if(rowsplits.size()){
        *this = simpleArray<T>(shape,rowsplits);
    }
    int size = sizeFromShape(shape);
    if (size != size_) {
        *this = simpleArray<T>(shape);
    } else if (size == size_) {
        shape_ = shape;
    }
}
template<class T>
size_t simpleArray<T>::getFirstDimension()const{
    if(!size_ || !shape_.size())
        return 0;
    return shape_.at(0);
}

template<class T>
T * simpleArray<T>::disownData() {
    T * dp = data_;
    data_ = 0;
    clear();
    return dp;
}

/*
 * Splits on first axis.
 * Returns the first part, leaves the second
 * for ragged it is the number of elements index - need to be consistent with the rowplits
 *
 * add function 'size_t getClosestSplitPoint(size_t splitnelements, bool down=True)'
 *
 * for ragged, the split point is the INDEX IN THE ROWSPLIT VECTOR!
 *
 */
template<class T>
simpleArray<T> simpleArray<T>::split(size_t splitindex) {
    simpleArray<T> out;
    if (!shape_.size() || (!isRagged() && splitindex > shape_.at(0))) {
        throw std::runtime_error(
                "simpleArray<T>::split: splitindex > shape_[0]");
    }
    if(isRagged() && splitindex >  rowsplits_.size())
        throw std::runtime_error(
                "simpleArray<T>::split: ragged split index out of range");


    //get split point for data
    ///insert rowsplit logic below
    size_t splitpoint = splitindex;
    if(isRagged()){
        splitpoint = rowsplits_.at(splitindex);
        for (size_t i = 2; i < shape_.size(); i++)
            splitpoint *= std::abs(shape_.at(i));
    }
    else{
        for (size_t i = 1; i < shape_.size(); i++)
            splitpoint *= std::abs(shape_.at(i));
    }


    size_t remaining = size_ - splitpoint;

    T * odata = new T[splitpoint];
    T * rdata = new T[remaining];

    memcpy(odata, data_, splitpoint * sizeof(T));
    memcpy(rdata, data_ + splitpoint, remaining * sizeof(T));
    if(!assigned_)
        delete data_;
    out.data_ = odata;
    data_ = rdata;
    ///insert rowsplit logic below
    out.shape_ = shape_;
    out.shape_.at(0) = splitindex;
    shape_.at(0) = shape_.at(0) - splitindex;
    if(isRagged()){
        out.rowsplits_ = std::vector<size_t> (rowsplits_.begin(),rowsplits_.begin()+splitindex+1);
        int outnelements = out.rowsplits_.at(out.rowsplits_.size()-1);
        rowsplits_ = std::vector<size_t> (rowsplits_.begin()+splitindex,rowsplits_.end());
        for(size_t i=0;i<rowsplits_.size();i++){
            rowsplits_.at(i)-=outnelements;
        }

        out.shape_ = out.shapeFromRowsplits();
        shape_ = shapeFromRowsplits();
    }
    ///
    out.size_ = sizeFromShape(out.shape_);
    size_ = sizeFromShape(shape_);
    return out;
}

/*
 * Merges along first axis
 */
template<class T>
void simpleArray<T>::append(const simpleArray<T>& a) {

    if (!data_ && size_ == 0) {
        *this = a;
        return;
    }
    if (shape_.size() != a.shape_.size())
        throw std::out_of_range(
                "simpleArray<T>::append: shape dimensions don't match");
    if(isRagged() != a.isRagged())
        throw std::out_of_range(
                "simpleArray<T>::append: can't append ragged to non ragged or vice versa");

    std::vector<int> targetshape;
    if (shape_.size() > 1 && a.shape_.size() > 1) {
        size_t offset = 1;
        if(isRagged())
            offset = 2;

        std::vector<int> highshape = std::vector<int>(shape_.begin() + offset,
                shape_.end());
        std::vector<int> ahighshape = std::vector<int>(a.shape_.begin() + offset,
                a.shape_.end());
        if (highshape != ahighshape) {
            throw std::out_of_range(
                    "simpleArray<T>::append: all shapes but first axis must match");
        }
        targetshape.push_back(shape_.at(0) + a.shape_.at(0));
        if(isRagged())
            targetshape.push_back(-1);
        targetshape.insert(targetshape.end(), highshape.begin(),
                highshape.end());
    } else {
        targetshape.push_back(shape_.at(0) + a.shape_.at(0));
    }

    T * ndata = new T[size_ + a.size_];
    memcpy(ndata, data_, size_ * sizeof(T));
    memcpy(ndata + size_, a.data_, a.size_ * sizeof(T));
    if(!assigned_)
        delete data_;
    data_ = ndata;
    size_ = size_ + a.size_;
    ///insert rowsplit logic below
    shape_ = targetshape;
    //recalculate -XxY part of the shape
    //append the row splits if dimensions match (- on same axis)
    ///
    if(isRagged()){
        //need copy in case this == &a
        auto ars = a.rowsplits_;

        size_t oldrssize = rowsplits_.size();
        int nelements = rowsplits_.at(oldrssize-1);
        size_t newrssize = oldrssize + ars.size()-1;
        rowsplits_.resize(newrssize);
        for(size_t i=0;i<ars.size();i++){
            rowsplits_.at(i+oldrssize-1) = ars.at(i) + nelements;
        }

        shape_ = shapeFromRowsplits();//last
    }
}

template<class T>
void simpleArray<T>::addToFile(FILE *& ofile) const {



    float version = DJCDATAVERSION;
    io::writeToFile(&version, ofile);
    io::writeToFile(&size_, ofile);
    size_t ssize = shape_.size();
    io::writeToFile(&ssize, ofile);
    io::writeToFile(&shape_[0], ofile, shape_.size());

    size_t rssize = rowsplits_.size();
    io::writeToFile(&rssize,  ofile);

    if(rssize){
        quicklz<size_t> iqlz;
        iqlz.writeCompressed(&rowsplits_[0],rssize , ofile);
    }
    quicklz<T> qlz;
    qlz.writeCompressed(data_, size_, ofile);

}

template<class T>
void simpleArray<T>::readFromFile(FILE *& ifile) {
    clear();

    float version = 0;
    io::readFromFile(&version, ifile);
    if(version != DJCDATAVERSION)
        throw std::runtime_error("simpleArray<T>::readFromFile: wrong format version");

    io::readFromFile(&size_, ifile);

    size_t shapesize = 0;
    io::readFromFile(&shapesize, ifile);
    shape_ = std::vector<int>(shapesize, 0);
    io::readFromFile(&shape_[0], ifile, shapesize);

    size_t rssize = 0;
    io::readFromFile(&rssize, ifile);
    rowsplits_ = std::vector<size_t>(rssize, 0);

    if(rssize){
        quicklz<size_t> iqlz;
        iqlz.readAll(ifile, &rowsplits_[0]);
    }
    quicklz<T> qlz;
    data_ = new T[size_];
    size_t nread = qlz.readAll(ifile, data_);
    if (nread != size_)
        throw std::runtime_error(
                "simpleArray<T>::readFromFile: expected and observed length don't match");

}
template<class T>
void simpleArray<T>::copyFrom(const simpleArray<T>& a) {

    if (&a == this) {
        return;
    }
    if (data_&& !assigned_)
        delete data_;
    data_ = new T[a.size_];
    memcpy(data_, a.data_, a.size_ * sizeof(T));

    size_ = a.size_;
    shape_ = a.shape_;
    rowsplits_ = a.rowsplits_;
    assigned_=false;
}

template<class T>
int simpleArray<T>::sizeFromShape(std::vector<int> shape) const {
    int size = 1;
    size_t previous=1;
    for (const auto s : shape){
        size *= std::abs(s);
        if(s<0)
            size/=previous*previous;
        previous=s;
    }
    return size;
}

template<class T>
std::vector<int> simpleArray<T>::shapeFromRowsplits()const{
    if(!isRagged()) return shape_;
    if(shape_.size()<2) return shape_;
    int nbatch = shape_.at(0);
    auto outshape = shape_;
    //rowsplits.size = nbatch+1
    outshape.at(1) = -nbatch * rowsplits_.at(rowsplits_.size()-1);
    return outshape;
}

template<class T>
void simpleArray<T>::checkShape(size_t ndims)const{
    //rowsplit ready due to definiton of shape
    if(ndims != shape_.size()){
        throw std::out_of_range("simpleArray<T>::checkShape: shape does not match dimensions accessed");
    }
}

template<class T>
void simpleArray<T>::checkSize(size_t idx)const{
    if(idx >= size_)
        throw std::out_of_range("simpleArray<T>::checkSize: index out of range");
}

template<class T>
void simpleArray<T>::checkRaggedIndex(size_t i, size_t j)const{
    if(i > rowsplits_.size()-1 || j >= rowsplits_.at(i+1)-rowsplits_.at(i))
        throw std::out_of_range("simpleArray<T>::checkRaggedIndex: index out of range");
}



// rowsplit support being added here (see whiteboard)
template<class T>
size_t simpleArray<T>::flatindex(size_t i, size_t j)const{
    size_t flat = 0;
    if(isRagged()){
        checkRaggedIndex(i,j);
        flat = rowsplits_.at(i)+j;}
    else{
        flat = j + shape_.at(1)*i;}
    return flat;
}

//this can also be ragged
template<class T>
size_t simpleArray<T>::flatindex(size_t i, size_t j, size_t k)const{
    size_t flat = 0;
    if(isRagged()){
        checkRaggedIndex(i,j);
        flat = k + shape_.at(2)*(rowsplits_.at(i)+j);}
    else{
        flat = k + shape_.at(2)*(j + shape_.at(1)*i);}
    return flat;
}
template<class T>
size_t simpleArray<T>::flatindex(size_t i, size_t j, size_t k, size_t l)const{
    size_t flat = 0;
    if(isRagged()){
        checkRaggedIndex(i,j);
        flat = l + shape_.at(3)*(k + shape_.at(2)*(rowsplits_.at(i)+j));}
    else{
        flat = l + shape_.at(3)*(k + shape_.at(2)*(j + shape_.at(1)*i));}
    return flat;
}
template<class T>
size_t simpleArray<T>::flatindex(size_t i, size_t j, size_t k, size_t l, size_t m)const{
    size_t flat = 0;
        if(isRagged()){
            checkRaggedIndex(i,j);
            flat = m + shape_.at(4)*(l + shape_.at(3)*(k + shape_.at(2)*(rowsplits_.at(i)+j)));}
        else{
            flat = m + shape_.at(4)*(l + shape_.at(3)*(k + shape_.at(2)*(j + shape_.at(1)*i)));}
    return flat;
}
template<class T>
size_t simpleArray<T>::flatindex(size_t i, size_t j, size_t k, size_t l, size_t m, size_t n)const{
    size_t flat = 0;
    if(isRagged()){
        checkRaggedIndex(i,j);
        flat = n + shape_.at(5)*(m + shape_.at(4)*(l + shape_.at(3)*(k + shape_.at(2)*(rowsplits_.at(i)+j))));}
    else{
        flat = n + shape_.at(5)*(m + shape_.at(4)*(l + shape_.at(3)*(k + shape_.at(2)*(j + shape_.at(1)*i))));}
    return flat;
}


//no row split support here!! needs to be added!
//relatively easy if dimension 1 is row split. other dimensions harder


template<class T>
size_t simpleArray<T>::sizeAt(size_t i)const{
    checkShape(2);
    if(!isRagged())
        return shape_.at(1);
    checkRaggedIndex(i,0);
    return rowsplits_.at(i+1)-rowsplits_.at(i);
}


template<class T>
T & simpleArray<T>::at(size_t i){
    checkShape(1);
    checkSize(i);
    return data_[i];
}

template<class T>
const T & simpleArray<T>::at(size_t i)const{
    checkShape(1);
    checkSize(i);
    return data_[i];
}

template<class T>
T & simpleArray<T>::at(size_t i, size_t j){
    checkShape(2);
    size_t flat = flatindex(i,j);
    checkSize(flat);
    return data_[flat];
}

template<class T>
const T & simpleArray<T>::at(size_t i, size_t j)const{
    checkShape(2);
    size_t flat = flatindex(i,j);
    checkSize(flat);
    return data_[flat];
}

template<class T>
T & simpleArray<T>::at(size_t i, size_t j, size_t k){
    checkShape(3);
    size_t flat = flatindex(i,j,k);
    checkSize(flat);
    return data_[flat];
}

template<class T>
const T & simpleArray<T>::at(size_t i, size_t j, size_t k)const{
    checkShape(3);
    size_t flat = flatindex(i,j,k);
    checkSize(flat);
    return data_[flat];
}

template<class T>
T & simpleArray<T>::at(size_t i, size_t j, size_t k, size_t l){
    checkShape(4);
    size_t flat = flatindex(i,j,k,l);
    checkSize(flat);
    return data_[flat];
}

template<class T>
const T & simpleArray<T>::at(size_t i, size_t j, size_t k, size_t l)const{
    checkShape(4);
    size_t flat = flatindex(i,j,k,l);
    checkSize(flat);
    return data_[flat];
}

template<class T>
T & simpleArray<T>::at(size_t i, size_t j, size_t k, size_t l, size_t m){
    checkShape(5);
    size_t flat = flatindex(i,j,k,l,m);
    checkSize(flat);
    return data_[flat];
}

template<class T>
const T & simpleArray<T>::at(size_t i, size_t j, size_t k, size_t l, size_t m)const{
    checkShape(5);
    size_t flat = flatindex(i,j,k,l,m);
    checkSize(flat);
    return data_[flat];
}

template<class T>
T & simpleArray<T>::at(size_t i, size_t j, size_t k, size_t l, size_t m, size_t n){
    checkShape(6);
    size_t flat = flatindex(i,j,k,l,m,n);
    checkSize(flat);
    return data_[flat];
}

template<class T>
const T & simpleArray<T>::at(size_t i, size_t j, size_t k, size_t l, size_t m, size_t n)const{
    checkShape(6);
    size_t flat = flatindex(i,j,k,l,m,n);
    checkSize(flat);
    return data_[flat];
}

}

#endif /* DJCDEV_DEEPJETCORE_COMPILED_INTERFACE_SIMPLEARRAY_H_ */
