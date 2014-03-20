// fast-lookup.cpp
//

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tr1/memory>
#include <cstring>
#include <iomanip>
#include <map>

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
    Equity(const char* equityName="") :
        _EquityName(equityName),
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

// Stub is a diagnostic aid, printing to stderr:
struct Stub  {
    std::ostream& operator << (const char* msg) {
        std::cerr << msg;
        return std::cerr;
    }
    std::ostream& operator << (const string& msg) {
        std::cerr << msg;
        return std::cerr;
    }
};

typedef shared_ptr<Equity> EquityPtr;



// The EquityMap class is an associative array which provides a fast-lookup container
// for Equity objects:
class EquityMap {
public:
    EquityMap() {
    }

    virtual ~EquityMap() {
    }

    typedef std::map<string,EquityPtr> MapT;
    typedef MapT::const_iterator const_iterator;
    const_iterator begin() const {
        return _Map.begin();
    }
    const_iterator end() const {
        return _Map.end();
    }

    void Insert(const EquityPtr& e) {
        _Map[e->GetEquityName()] = e;
    }

    // Finds an Equity object by name.  Throws a domain_error if not found.
    EquityPtr  FindByEquityName(const char* name) const {
        MapT::const_iterator it=_Map.find(name);
        if (it==_Map.end()) {
            throw new std::domain_error("No such equity name");
        }
        return it->second;
    }

private:
    // We really don't want people accidentally copying this.  It's expensive.  If that operation
    // is needed, we'll add a Clone() method.
    EquityMap(const EquityMap& );
    void operator = (const EquityMap&);

    MapT _Map;


};


// Split a string on a single-char delimiter, and populate *this
// with the resulting tokens:
class StringSplitter : public std::vector<string> {
public:
    void Insert( std::stringstream& ss, char delimiter ) {
        string cur;
        while (std::getline(ss,cur,delimiter)) {
            this->push_back(cur);
        }
    }
    StringSplitter(const char* text, char delimiter) {
        std::stringstream ss(text);
        Insert(ss , delimiter);
    }

    StringSplitter(const std::string& text, char delimiter) {
        std::stringstream ss(text);
        Insert( ss , delimiter);
    }

};

// Trim leading whitespace from a string.  
//   Usage:
//      string newStr =  LTrim(old_string);
//
struct LTrim {
    LTrim(const string& orig) {
        size_t ix=orig.find_first_not_of(" \t\n\r\b");
        if (ix==string::npos) 
            _Result=orig;
        else
            _Result=orig.substr(ix);
        
    }
    string _Result;
    operator const string&() const {
        return _Result;
    }
};

// ParseString offers a few conversion functions which return false if
// 'input' doesn't validate for the 'target' type.  Otherwise, the string
// is converted and target is updated.
class ParseString {
public:
    ParseString(const string& xinput, long long& target) : _Ok(false) {
        string input = LTrim( xinput );
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

    ParseString(const string& xinput, double & target) : _Ok(false) {
        string input = LTrim( xinput );
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
    double cap=(double)val.GetMarketCap()/(double)1000000.0;
    output
            << "code: " << val.GetEquityName()
            << " description: " << val.GetDescription()
            << std::fixed << std::setprecision(3)
            << " last price: " << val.GetPrice()
            << " market cap: " << cap << " Million "
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


    void printBadRecordMsg(const string& context) const {
        cout << "Not found\n";
        Stub() << "Failed at " << context << std::endl;
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
            printBadRecordMsg(inputString);
            return false;
        }

        // The EquityName field requires validation:
        if (! parse_EquityName( splitter[ F_EquityName ], result._EquityName )) {
            printBadRecordMsg(inputString);
            return false;
        }

        // We have no particular requirements for description validation, so a
        // simple assignment should serve:
        result._Description = splitter[ F_Description ];

        // Parse the MarketCap field:
        if (! ParseString( splitter[ F_MarketCap ], result._MarketCap )) {
            printBadRecordMsg(inputString);
            return false;
        }

        // Parse the Price field:
        if (! ParseString( splitter[ F_Price ], result._Price )) {
            printBadRecordMsg(inputString);
            return false;
        }

        // Parse the PE_ratio field:
        if (! ParseString( splitter[ F_PE_ratio ], result._PE_ratio )) {
            printBadRecordMsg(inputString);
            return false;
        }
        return true;
    }
};


// Given an input stream or filename, this class parses the stream and fills
// the caller's EquityMap with Equity objects.   If a record error occurs, we
// just print an error and skip it.  For other errors, we throw std::runtime_error
//
class EquityLoader {
public:
    EquityLoader( std::istream& input, EquityMap& output ) {

        EquityTextFactory fact;

        string header;
        if (!std::getline(input,header)) {
            throw std::runtime_error("No header line in input");
        }
        // Loop the input stream until end-of-file:
        string line;

        while ( std::getline(input, line) ) {
            EquityPtr newEquity=fact.ParseEquity( line.c_str() );
            if (!newEquity) {
                // If parse failed, keep going...
                continue;

            }

            // Save our new Equity object in the caller's collection:
            output.Insert(newEquity);

            Stub() << "Inserted " << newEquity->GetEquityName() << std::endl;
        }

    }
};

// EquityService owns the EquityMap and provides application-level query
// interfaces.
//
class EquityService {
public:
    EquityService() {
    }

    // initialize() loads all security data from stdin, filling _Map:
    void initialize(std::istream& input=std::cin) {
        try {
            EquityLoader( input, _Map);
        }
        catch (...) {
            std::cerr << "EquityService.initialize() failed" << std::endl;
        }
    }

    // Returns an EquityPtr containing attributes of the given equity.  EquityPtr
    // will be null if equityName not found.
    EquityPtr getSecurityInfo(const char* equityName) {

        try {
            return _Map.FindByEquityName(equityName);
        }
        catch (std::exception e) {
            std::cerr << e.what() << std::endl;
        }
        return EquityPtr();

    }

    // Returns all security names, ordered alphabetically:
    std::string allSecurityCodes() const {

        // Our Map stores its elements indexed by equity name, and the
        // underlying comparison operator uses the std::string operator '<'.
        // So all we have to do is build a string with one equity name per
        // line.

        std::stringstream ss;
        EquityMap::const_iterator it=_Map.begin();
        while (it != _Map.end()) {
            ss << it->first << "\n";
            ++it;
        }
        return ss.str();
    }

private:
    EquityMap _Map;
};


#define _COMPILE_UNIT_TESTS
#ifdef _COMPILE_UNIT_TESTS

class test_EquityParser {
public:
    test_EquityParser() {
        EquityTextFactory fact_00;
        try {
            EquityPtr ptr=fact_00.ParseEquity("IBMUS|International Business Machines|198657057012|182.95|11.18");
            std::cerr <<  *ptr << std::endl;
        }
        catch (std::exception e) {
            std::cerr << e.what() << std::endl;
        }
    }
};

class test_EquityService {
public:
    test_EquityService() {
        EquityService srv;

        // Test loading the EquityService with a known data stream:
        std::ifstream test_input("test_cases/input000.txt");
        if (! test_input.is_open()) {
            throw std::runtime_error("Can't open input000.txt");
        }
        const int input000_record_count=17;
        srv.initialize(test_input); 

        {
            // Make sure we can retrieve a security:
            EquityPtr p=srv.getSecurityInfo("MSFTUS");
            if ( !p || (p->GetEquityName() != "MSFTUS" ))
                throw std::runtime_error("Can't find MSFTUS");
        }

        {
            // Verify that we can retrieve all security codes:
            string allCodes=srv.allSecurityCodes();
            // How many records did we get?
            StringSplitter sp( allCodes, '\n');
            if (sp.size() != input000_record_count)
                throw std::runtime_error("Invalid record count for allSecurityCodes()");
        
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

#ifdef _COMPILE_UNIT_TESTS
    if ( args.RunUnitTests ) {
        test_EquityParser test_00;
        test_EquityService test_01;
    }
#endif

}
