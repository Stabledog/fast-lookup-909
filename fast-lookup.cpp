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

// Base class for Equity filter functor types.
struct EquityFilter {
    virtual const Equity& Compare(const Equity& left, const Equity& right) const {
        return left;
    }
    virtual bool Select(const Equity& elem) const {
        return false;
    }
};


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

    // Iteratively invokes a selection filter on each element in the collection to
    // produce a selected-items collection.  Returns number of elements added
    // to the 'result' collection.
    int SelectByFilter( const EquityFilter& filter, EquityMap & result) const {
        // If the map is empty, return an empty pointer:
        if ( _Map.size()== 0)
            return 0;

        int nAdded=0;
        for (const_iterator it= begin(); it !=end(); ++it) {
            if (filter.Select(*it->second)) {
                ++nAdded;
                result.Insert(it->second);
            }
        }
        return nAdded;
    }

    // Iteratively compares each element in the collection using a caller-supplied
    // comparison filter.  Returns the element which the filter determines to be
    // best-fit:
    EquityPtr FindByCompareFilter( const EquityFilter& filter) const {
        // Our initial first-item is the first item in the collection.
        if ( _Map.size()== 0)
            return EquityPtr();
        // In a 1-element collection, the compare filter is irrelevant:
        if (_Map.size()==1)
            return _Map.begin()->second;

        EquityPtr cur=_Map.begin()->second;
        for (const_iterator it= ++begin(); it !=end(); ++it) {
            const Equity& selected = filter.Compare(*cur, *it->second);
            if ( &selected != &*cur )
                // The filter chose the right-side equity:
                cur=it->second;

        }
        return cur;
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

        try {
            while ( std::getline(input, line) ) {
                try {
                    EquityPtr newEquity=fact.ParseEquity( line.c_str() );
                    if (!newEquity) {
                        // If parse failed, keep going...
                        continue;

                    }

                    // Save our new Equity object in the caller's collection:
                    output.Insert(newEquity);

                    Stub() << "Inserted " << newEquity->GetEquityName() << std::endl;
                }
                catch (std::exception e) {
                    std::cerr << e.what() << std::endl;
                }
            }
        }
        catch (std::exception e) {
            std::cerr << e.what() << std::endl;
        }

    }
};


// Compares two Equity objects, returning the one with the lowest P/E.  If
// the objects have the same P/E, it returns the one with the lowest price.
class LowestPE_filter : public EquityFilter {
    const Equity& Compare(const Equity& left, const Equity& right) const {
        double l_pe=left.GetPE_ratio(),
               r_pe=right.GetPE_ratio();
        if (l_pe < r_pe)
            return left;
        if (r_pe < l_pe)
            return right;
        // Fall-through: the P/E ratios are equal, so return the one
        // with the lowest price:
        if ( left.GetPrice() < right.GetPrice() ) {
            return left;
        }
        return right;
    }
};

// Selects Equity objects by comparing their P/E ratio to a caller-defined [min,max] range.
class PE_RangeFilter : public EquityFilter {
public:
    PE_RangeFilter( double minPE, double maxPE ): _minPE(minPE),_maxPE(maxPE) {
    }

    bool Select( const Equity& elem ) const {
        double pe=elem.GetPE_ratio();
        if ((pe < _minPE) || (pe > _maxPE))
            return false;
        return true;
    }

private:
    double _minPE;
    double _maxPE;

};

// EquityService owns the EquityMap and provides application-level query
// interfaces.
//
class EquityService {
public:
    EquityService() {
    }

    // initialize() loads all security data from stdin, filling _Map:
    bool initialize(std::istream& input=std::cin) {
        try {
            EquityLoader( input, _Map);
        }
        catch (...) {
            std::cerr << "EquityService.initialize() failed" << std::endl;
            return false;
        }
        return true;
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
    string allSecurityCodes() const {

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

    // Returns the name of the security with the lowest P/E ratio:
    string lowestPE() const {
        const EquityPtr result=_Map.FindByCompareFilter(  LowestPE_filter() );
        if (result) {
            return result->GetEquityName();
        }
        return "";
    }

    // Returns the number of Equity objects whose P/E values are in the range specified,
    // adding them to caller's collection.
    int getPERange( double min_pe, double max_pe, EquityMap& result ) const {
        return _Map.SelectByFilter( PE_RangeFilter( min_pe, max_pe), result  );
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
            // Select all the equities in given range:
            EquityMap selected;
            int count=srv.getPERange( 6.0, 15.0, selected );
            if (count != 11) {
                throw std::runtime_error("Incorrect number of P/E-range matches");
            }
        }

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
        {
            // Find the lowest P/E:
            string lowestPe=srv.lowestPE();
            Stub() << "Lowest P/E:" << lowestPe << std::endl;
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
                // We're expecting an input filename.  If we don't get it, stdin is the default:
                InputFile=argv[i];
            }
        }
    }

    bool RunUnitTests;
    string InputFile;

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
#ifdef _COMPILE_UNIT_TESTS
        test_EquityParser test_00;
        test_EquityService test_01;
#else
        throw std::runtime_error( "Unit tests are not enabled for this build." );
#endif
    }
    else
    {   // Main line logic:
        try {
            EquityService srv;
            std::ifstream inputFile( args.InputFile.length() ? args.InputFile : "/dev/zero");

            // We'll either use stdin or the input file, if the latter was specified:
            std::istream & input( args.InputFile.length() ? inputFile :  std::cin );

            // Load from stdin to EquityService:
            bool ok=srv.initialize( input );
            if (!ok) {
                std::cerr << "EquityService.initialize() failed" << std::endl;
                return 1;
            }

            // Print out these securities:
            const char* printItems[] = {"IBMUS","AAPLUS", "AALLN", "30HK"};
            for (int i = 0; i < sizeof(printItems)/sizeof(*printItems); ++i) {
                cout << "Lookup for Code " << printItems[i] << std::endl;
                EquityPtr e=srv.getSecurityInfo( printItems[i] );
                cout << *e << std::endl;
            }

            cout << "All codes:" << std::endl;
            // Print out all security codes:
            cout << srv.allSecurityCodes() << std::endl;

            // Print the equity with the lowest P/E:
            {
                string lowestPE=srv.lowestPE();
                EquityPtr eq=srv.getSecurityInfo(lowestPE.c_str());
                cout << "Lowest P/E is " << std::setprecision(3) << eq->GetPE_ratio() << " from code " << lowestPE << std::endl;
            }

            {
                // Fill and print an EquityMap containing the securities whose
                // P/E is between 6 ands 16:
                EquityMap selected;

                cout << "Get equity objects whose P/E is between 6 and 15" << std::endl;
                srv.getPERange(6.0,15.0,selected);

                cout << "The following have P/E between 6.000 and 15.000" << std::endl;

                for (EquityMap::const_iterator it= selected.begin();
                        it != selected.end();
                        ++it) {
                    cout << *it->second << std::endl;

                }
            }

        }
        catch (std::exception e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    }
    return 0;
}
