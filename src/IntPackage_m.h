//
// Generated file, do not edit! Created by nedtool 5.5 from IntPackage.msg.
//

#ifndef __INTPACKAGE_M_H
#define __INTPACKAGE_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0505
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>IntPackage.msg:19</tt> by nedtool.
 * <pre>
 * //
 * // TODO generated message class
 * //
 * packet IntPackage
 * {
 *     int error_value;
 *     int total_value;
 *     int fuzzy_value;
 * }
 * </pre>
 */
class IntPackage : public ::omnetpp::cPacket
{
  protected:
    int error_value;
    int total_value;
    int fuzzy_value;

  private:
    void copy(const IntPackage& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const IntPackage&);

  public:
    IntPackage(const char *name=nullptr, short kind=0);
    IntPackage(const IntPackage& other);
    virtual ~IntPackage();
    IntPackage& operator=(const IntPackage& other);
    virtual IntPackage *dup() const override {return new IntPackage(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getError_value() const;
    virtual void setError_value(int error_value);
    virtual int getTotal_value() const;
    virtual void setTotal_value(int total_value);
    virtual int getFuzzy_value() const;
    virtual void setFuzzy_value(int fuzzy_value);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const IntPackage& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, IntPackage& obj) {obj.parsimUnpack(b);}


#endif // ifndef __INTPACKAGE_M_H

