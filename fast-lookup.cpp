// fast-lookup.cpp
//

#include <vector>
#include <iostream>
#include <sstream>
#include <tr1/memory>
#include <cstring>

using std::cout;
using std::string;
using std::tr1::shared_ptr;


//
// Equity class
//
// This class represents the properties of a single equity.
class Equity {

public:
    // Construct an Equity from its component parts:
    Equity(
        const char* equityName,  //  Name/symbol
        const char* description, //  Plain-text description
        long long   marketCap,   //  Market capitalization, USD$
        double      price,       //  Price, US$
        double      PE_ratio     //  P/E ratio
    ) :
        _EquityName(equityName),
        _Description(description),
        _MarketCap(marketCap),
        _Price(price),
        _PE_ratio(PE_ratio) {
    }

    // Default constructor zeroes out the POD fields only:
    Equity() :
        _MarketCap(0),
        _Price(0),
        _PE_ratio(0) {
    }

    // Returns the equity name/symbol:
    const string & GetEquityName() const {
        return _EquityName;
    }

    // Returns a plain-text description of the equity:
    const string & GetDescription() const {
        return _Description;
    }

    // Returns the market capitalization for this equity in USD$:
    long long GetMarketCap() const {
        return _MarketCap;
    }

    // Returns the current price of this equity in USD$:
    double GetPrice() const {
        return _Price;
    }

    // Returns the P/E (Price-to-Earnings) ratio at the current price:
    double GetPE_ratio() const {
        return _PE_ratio;
    }

private:
    string      _EquityName;
    string      _Description;
    long long   _MarketCap;
    double      _Price;
    double      _PE_ratio;

    friend class EquityTextFactory;

};


// Split a string on a single-char delimiter, and populate *this
// with the resulting tokens:
class StringSplitter : public std::vector<string> {
public:
    StringSplitter(const char* text, char delimiter) {
        std::stringstream ss(text);
        string cur;
        while (std::getline(ss,cur,delimiter)) {
            this->push_back(cur);
        }
    }

};


// ParseString offers a few conversion functions which return false if
// 'input' doesn't validate for the 'target' type.  Otherwise, the string
// is converted and target is updated.
class ParseString {
public:
    ParseString(const string& input, long long& target) : _Ok(false) {
        // Make sure we only have 0-9 in the input:
        if (input.find_first_not_of("0123456789") != std::string::npos) {
            return;  //   Oops.  One or more non-digits!
        }
        try {
            target = std::stoll( input );
        }
        catch ( std::out_of_range ) {
            return;
        }
        _Ok=true;  // We're happy now.
    }

    ParseString(const string& input, double & target) : _Ok(false) {
        // Make sure we only have 0-9 or '.' in the input:
        if (input.find_first_not_of("0123456789.") != std::string::npos) {
            return;  //   Oops.  One or more invalid chars!
        }
        try {
            target = std::stod( input );
        }
        catch ( std::out_of_range ) {
            return;
        }
        _Ok=true;  // We're happy now.
    }

    operator bool() const {
        return _Ok;
    }

private:
    bool _Ok;
};


//  Prints an Equity object to a stream:
std::ostream& operator << (std::ostream& output, const Equity& val) {
    output
            << "code: " << val.GetEquityName()
            << " description: " << val.GetDescription()
            << " last price: " << val.GetPrice()
            << " market cap: " << val.GetMarketCap()  << " Million "
            << " P/E: " << val.GetPE_ratio()
            ;
}


// This class creates Equity objects by parsing a line of text formatted as
// follows:
//
// HEADER:Code|Description|Market Cap|Price|P/E Ratio
// IBMUS|International Business Machines|198657057012|182.95|11.18
//
//
class EquityTextFactory {
public:
    typedef shared_ptr<Equity> EquityPtr;


    // Our field schema:
    enum { F_EquityName=0, F_Description, F_MarketCap, F_Price, F_PE_ratio, F_COUNT };

    // Parse an Equity object from a line of text, using the field schema
    // described above.   If the text cannot be parsed, the EquityPtr returned
    // will be null and "Not found" is printed to stdout.
    EquityPtr ParseEquity(const char* inputString) {
        Equity tmp;
        if (ParseEquity(inputString, tmp)) {
            return EquityPtr(new Equity( tmp ));
        }
        return EquityPtr();
    }

private:


    void printBadRecordMsg() const {
        cout << "Not found";
    }

    // Validates the EquityName against business rules: it must conform to:
    //      1.  charset: [A-Z0-9]+
    //      2.  length: 1 <= length <= 6 chars
    bool parse_EquityName( const string& rawField, string& target ) {

        const size_t EquityName_MaxLen=6;

        // Check the character set:
        if ( rawField.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != std::string::npos) {
            return false;
        }

        if (
            (rawField.size() < 1) ||   // Bounds-check the length.
            ( rawField.size() > EquityName_MaxLen )
        )
            return false;

        // Having passed validation, now we can assign the result:
        target=rawField;

        return true;

    }


    bool ParseEquity(const char* inputString, /*out*/ Equity& result) {

        // Split the input string into tokens, using '|' as a delimiter.
        StringSplitter splitter(inputString,'|');

        // Sanity-check the number of tokens:
        if ( splitter.size() != (size_t)F_COUNT ) {
            printBadRecordMsg();
            return false;
        }

        // The EquityName field requires validation:
        if (! parse_EquityName( splitter[ F_EquityName ], result._EquityName )) {
            printBadRecordMsg();
            return false;
        }

        // We have no particular requirements for description validation, so a
        // simple assignment should serve:
        result._Description = splitter[ F_Description ];

        // Parse the MarketCap field:
        if (! ParseString( splitter[ F_MarketCap ], result._MarketCap )) {
            printBadRecordMsg();
            return false;
        }

        // Parse the Price field:
        if (! ParseString( splitter[ F_Price ], result._Price )) {
            printBadRecordMsg();
            return false;
        }

        // Parse the PE_ratio field:
        if (! ParseString( splitter[ F_PE_ratio ], result._PE_ratio )) {
            printBadRecordMsg();
            return false;
        }
        return true;
    }
};

#define _COMPILE_UNIT_TESTS
#ifdef _COMPILE_UNIT_TESTS

class test_EquityParser {
public:
    test_EquityParser() {
        EquityTextFactory fact_00;
        {
            EquityTextFactory::EquityPtr ptr=fact_00.ParseEquity("IBMUS|International Business Machines|198657057012|182.95|11.18");
            std::cerr <<  *ptr;
        }
    }
};

#endif

// Our command-args parser:
class ParseArgs {
public:
    ParseArgs(int argc, char* argv[]) :
        RunUnitTests(false) {
        for (int i = 1; i < argc ; ++i) {
            if ( match(argv[i], "-t" )) {
                RunUnitTests=true;
            }
            else {
                std::cerr << "Unknown argument: " << argv[i] << std::endl;
            }
        }
    }

    bool RunUnitTests;
    bool match( const char* left, const char* right) const {
        if (!left || !right) {
            return false;
        }
        return strcmp(left,right)==0;
    }
};

int main(int argc, char* argv[]) {
    // main() is our test driver:
    //
    //  Args:  -t:  Run unit tests.
    //

    ParseArgs args(argc,argv);

    if ( args.RunUnitTests ) {
        test_EquityParser test_00;
    }

}
