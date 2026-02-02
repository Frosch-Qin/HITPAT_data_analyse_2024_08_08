// create a class for register
// how to use this class see the test_reg() function


#include <gmp.h> // for big number
class REG
{
public:
    mpz_t mpzval;
    double douval; // douval is the double value of the mpzval
    int bitsize; // bitsize is the bitsize of the mpzval, the signed bit is not included

    REG();                        // constructor
    REG(mpz_t);                   // constructor
    REG(int);                     // constructor
    //~REG() { mpz_clear(mpzval); } // destructor

    void set_mpzval(int);
    void set_mpzval(mpz_t);
    void set_mpzval(REG);
    int getsize(void);
    double get_douval(void);
    REG operator+(const REG &);
    REG operator-(const REG &);
    REG operator*(const REG &);
    REG operator/(const REG &);
    REG operator=(const REG &);
    REG sqrt();
    REG reglog(int); // int is for point_bit
    //leftshift, rightshift and mask are applied to the mpzval itself
    REG leftshift(int);
    REG rightshift(int);
    void mask(int); // the mask, input is bitsize of the number, the signed bit is not included
};

void REG::mask(int x)
{
    mpz_t mask;
    mpz_init(mask);
    mpz_ui_pow_ui(mask, 2, x);
    mpz_sub_ui(mask, mask, 1);
    //
    if (mpz_cmp_ui(this->mpzval, 0) > 0)
    {
        if (mpz_cmp(this->mpzval, mask) > 0)
            mpz_and(this->mpzval, this->mpzval, mask);
    }
    else if (mpz_cmp_si(this->mpzval, -std::pow(2, x)) < 0)
    {

        int temp = this->douval;
        temp = -temp - 1;
        this->set_mpzval(temp);
        mpz_and(this->mpzval, this->mpzval, mask);
        get_douval();
        temp = this->douval;
        temp = -(temp + 1);
        this->set_mpzval(temp);
    }
    mpz_clear(mask);
    this->getsize();
    this->get_douval();
}
REG REG::reglog(int point_bit)
{
    REG result;
    double input;
    double output;
    input = this->douval;
    output = std::log(input);
    output = int(output * pow(2, point_bit));
    int temp = output;
    result.set_mpzval(temp);
    return result;
}

REG REG::sqrt()
{
    REG result;
    mpz_sqrt(result.mpzval, this->mpzval);
    result.getsize();
    result.get_douval();
    return result;
}

REG REG::leftshift(int x)
{
    REG result;
    mpz_mul_2exp(result.mpzval, this->mpzval, x);
    result.getsize();
    result.get_douval();
    return result;
}

REG REG::rightshift(int x)
{
    // two's complement rightshift
    REG result;
    mpz_fdiv_q_2exp(result.mpzval, this->mpzval, x);
    result.getsize();
    result.get_douval();
    return result;
}

REG REG::operator+(const REG &x)
{
    REG result;
    mpz_add(result.mpzval, this->mpzval, x.mpzval);
    result.getsize();
    result.get_douval();
    return result;
}

REG REG::operator-(const REG &x)
{
    REG result;
    mpz_sub(result.mpzval, this->mpzval, x.mpzval);
    result.getsize();
    result.get_douval();
    return result;
}

REG REG::operator*(const REG &x)
{
    REG result;
    mpz_mul(result.mpzval, this->mpzval, x.mpzval);
    result.getsize();
    result.get_douval();
    return result;
}

REG REG::operator/(const REG &x)
{
    REG result;
    mpz_tdiv_q(result.mpzval, this->mpzval, x.mpzval);
    result.getsize();
    result.get_douval();
    return result;
}

REG REG::operator=(const REG &x)
{
    mpz_init_set(this->mpzval, x.mpzval);
    this->getsize();
    this->get_douval();
    return *this;
}

REG::REG(mpz_t x)
{
    mpz_init(mpzval);
    mpz_set(mpzval, x);
    getsize();
    get_douval();
}
REG::REG()
{
    mpz_init(mpzval);
    mpz_set_si(mpzval, 0);
    getsize();
    get_douval();
}
REG::REG(int x)
{
    mpz_init(mpzval);
    mpz_set_si(mpzval, x);
    getsize();
    get_douval();
}
void REG::set_mpzval(int x)
{
    mpz_init_set_si(mpzval, x);
    getsize();
    get_douval();
}
void REG::set_mpzval(mpz_t x)
{
    mpz_init_set(mpzval, x);
    getsize();
    get_douval();
}
void REG::set_mpzval(REG x)
{
    mpz_init_set(mpzval, x.mpzval);
    getsize();
    get_douval();
}
int REG::getsize(void)
{
    this->bitsize = mpz_sizeinbase(this->mpzval, 2);
    return bitsize;
}
double REG::get_douval(void)
{
    this->douval = mpz_get_d(this->mpzval);
    return douval;
}


//write a accumulate REG class

class WATCH_REG
{
    public:
    REG temp;
    REG Sum;
    REG Max;
    REG Min;
    WATCH_REG(){};
    void set(WATCH_REG x){
        this->Sum.set_mpzval(x.Sum);
        this->Max.set_mpzval(x.Max);
        this->temp.set_mpzval(x.temp);
        this->Min.set_mpzval(x.Min);
    }
    template<typename T> //T is for REG, int or mpz_t
    void accumulate(T);
    template<typename T> //T is for REG, int or mpz_t
    void setvalue(T x)
    {
        this->temp.set_mpzval(x);
        if (this->Max.douval < this->temp.douval){
            this->Max.set_mpzval(this->temp);
        }
        if (this->Min.douval > this->temp.douval){
            this->Min.set_mpzval(this->temp);
        }
    }
    void clear()
    {
        Sum.set_mpzval(0);
        Max.set_mpzval(0);
        temp.set_mpzval(0);
        Min.set_mpzval(0);
    }
    WATCH_REG leftshift(int x)
    {
        WATCH_REG result;
        result.Sum.set_mpzval(this->Sum.leftshift(x));
        result.Max.set_mpzval(this->Max.leftshift(x));
        result.temp.set_mpzval(this->temp.leftshift(x));
        result.Min.set_mpzval(this->Min.leftshift(x));
        return result;
    }
}; 
template<typename T>
void WATCH_REG::accumulate(T x)
{
    this->temp.set_mpzval(x);
    this->Sum.set_mpzval(Sum + x);
    if ( this->Max.douval <  this->temp.douval)
    {
         this->Max.set_mpzval(this->temp);
    }
    if ( this->Min.douval >  this->temp.douval)
    {
         this->Min.set_mpzval(this->temp);
    }
}


// the code for test this class

int test_reg()
{
    REG a; // after REG declaration, it can only be assigned correctly by set_mpzval()
    REG b;

    std::cout << "test log" << std::endl;
    for (int i = 13; i < 127; i++)
    {
        a.set_mpzval(i);
        b.set_mpzval(a.reglog(12));
        std::cout << i << " " << std::log(i) << " " << b.douval / std::pow(2, 12) << std::endl;
    }
    std::cout << "test sqrt" << std::endl;
    for (int i = 0; i < 10; i++)
    {
        a.set_mpzval(i);
        b.set_mpzval(a.sqrt());
        std::cout << i << " " << std::sqrt(i) << " " << b.douval << std::endl;
    }

    std::cout << "test mask" << std::endl;
    for (int i = -10; i < 10; i++)
    {
        a.set_mpzval(i);
        a.mask(2);
        std::cout << i << " " << a.douval << std::endl;
    }

    std::cout << "test leftshift" << std::endl;
    for (int i = -10; i < 10; i++)
    {
        a.set_mpzval(i);
        b.set_mpzval(a.leftshift(5));
        std::cout << i << " " << b.douval << std::endl;
    }

    std::cout << "test rightshift" << std::endl;
    for (int i = -10; i < 10; i++)
    {
        a.set_mpzval(i);
        b.set_mpzval(a.rightshift(5));
        std::cout << i << " " << b.douval << std::endl;
    }

    std::cout << "test add" << std::endl;
    for (int i = -10; i < 10; i++)
    {
        a.set_mpzval(i);
        b.set_mpzval(i);
        a.set_mpzval(a + b);
        std::cout << i << " " << a.douval << std::endl;
    }

    std::cout << "test sub" << std::endl;
    for (int i = -10; i < 10; i++)
    {
        a.set_mpzval(i);
        b.set_mpzval(i);
        a.set_mpzval(a - b);
        std::cout << i << " " << a.douval << std::endl;
    }

    std::cout << "test mul" << std::endl;
    for (int i = -10; i < 10; i++)
    {
        a.set_mpzval(i);
        b.set_mpzval(i);
        a.set_mpzval(a * b);
        std::cout << i << " " << a.douval << std::endl;
    }

    std::cout << "test div" << std::endl;
    for (int i = 5; i < 10; i++)
    {
        a.set_mpzval(i);
        b.set_mpzval(2);
        a.set_mpzval(a / b);
        std::cout << i << " " << a.douval << std::endl;
    }

    std::cout << "test accumulate reg" << std::endl;
    WATCH_REG acc;
    for (int i = 0; i < 10; i++)
    {
        a.set_mpzval(-i);
        acc.accumulate(a);
        if (i == 5)
        {
            acc.clear();
            std::cout << "clear WATCH_REG" << std::endl;
        }
        std::cout << i << " " << acc.Sum.douval << " " << acc.Max.douval  << "  " << acc.Min.douval << std::endl;
    }

    return 0;
}