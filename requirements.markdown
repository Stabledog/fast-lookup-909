https://www.hackerrank.com/tests/17gm546in/questions/13qttl8k7
pwd: de4f2d3a
les.matheson@gmail.com



# Equity - Fast Lookup Service
------------------------------

## Introduction and review criteria
-----------------------------------
 

Your code will be the starting point for a service in a n-tiered architecture. The role of this service is to provide access to equity security attributes and fields. Callers will give an equity code (defined below) and the service will provide access to the attributes of the security. When coding, consider all aspects that will keep your code robust, scalable, and maintainable.

The code you create in your assignment will include the following components described below.

 


## Equity C++ class:
--------------------

    Contains equity security attributes such as name, description, current price, market cap, P/E, etc.
    These are just what are considered to be the main summary attributes of the equity security, and, by design, they are expected to be no bigger than 1KB. (For this assignment, they can be much smaller, with just a couple of strings and doubles.)

 


## EquityService C++ class:
---------------------------

    For performance reasons this service caches all data in memory. It’s designed to handle 100,000 securities with room to grow in the future to handle 500,000, if needed, while still keeping all information in memory.

    * Implement the method “initialize” that gets securities and their attributes from STDIN. The service will hold on to these data by means of the EquityMap class.

    * Implement the method “getSecurityInfo”: It will take an equity code and return the security attributes. Examples of equity codes: “IBMUS”, “AAPLUS”, “AALLN”, “30HK”, etc. Keys have at most 6 characters, and are composed of uppercase letters (ASCII only) or the digits 0-9. The method returns an object of the Equity class.

    * Implement the method “allSecurityCodes” that returns all security codes in alphabetical order.

    * Implement the method “lowestPE” that returns the security code of the security with the lowest P/E. If two or more securities have the same P/E exactly, return the one of them with the lowest price.

    * Implement the method "getPRRange(min_pe, max_pe)" that returns all objects whose P/E is between min_pe and max_pe. The returned entries should be in alphabetical order of equity code.

    Note that none of these methods print out values, they just pass back their results through their return values.

 

## EquityMap C++ class:
------------------------

    Design this class optimized for fast access to each one of potentially thousands of different companies--each identified by its equity code.



## Definition of Provided Input
--------------------------------

    For this exercise, input file is from STDIN and is delimited and has the following format: Code | Description | Market Cap | Price | P/E Ratio
    If there is any data error from input file, the line should be skipped and you should print "Input invalid".

 

main() function testing harness will consistent of these steps:
---------------------------------------------------------------

    * Parse the input into a several instances of the Equity class, and pass them to EquityService::initialize(), input file is '|' delimited and has the following format: Code | Description | Market Cap | Price | P/E Ratio
    
    * Call EquityService:: getSecurityInfo for “IBMUS”, “AAPLUS”, “AALLN”, and “30HK” and print out each returned object.
    * Call EquityService:: allSecurityCodes and print out those codes
    * Call EquityService:: lowestPE and print out that code
    * Call EquityService::getPERange() to print out all objects whose P/E is between 6 and 15

 

 

## Error Checking
-----------------
 

    Your code should have error checking. When there is no record found for the passed input code, your code should simply print "Not found".
    This test has 3 test cases, the first of which is shown below. The other 2 test cases contain error conditions, and are not visible to you. This tool requires an exact match of your output to the saved output file in order for you to get full credit, however this could be difficult to achieve, as an extra space or carriage return may be treated as error.  We will be reviewing your results manually, so outputting an error message should be sufficient. 
    When you print numeric value, always use 3 decimal places for the precision.

 

Sample Input:
-------------
 

HEADER:Code|Description|Market Cap|Price|P/E Ratio
IBMUS|International Business Machines|198657057012|182.95|11.18
AALLN|ANGLO AMERICAN PLC|22153061345|1589|0.1
AAPLUS|Apple Inc|479328136276|537.37|13.32
30HK|ABC COMMUNICATIONS HLDGS LTD| 261544852|0.158|0.11
857HK|PETROCHINA CO-H|1744803856384|7.95|9.56
5HK|HSBC HLDGS PLC|1597769646080|84.849998|12.24
1398HK|IND & COMM BK-H|1595123040256|4.84|4.720
941HK|CHINA MOBILE|1482285121536|73.699997|8.910
XOMUS|EXXON MOBIL CORP|413785620480|94.07|12.830
MSFTUS|MICROSOFT CORP|312081481728|37.419998|13.2048
JNJUS|JOHNSON&JOHNSON|259290185728|92.169998|16.773
GEUS|GENERAL ELECTRIC|258498772992|25.65|15.38766
WMTUS|WAL-MART STORES|241582768128|75.330002|14.40
CVXUS|CHEVRON CORP|220837953536|112.709999|10.289
JPMUS|JPMORGAN CHASE|217192349696|58.490002|8.42
PGUS|PROCTER & GAMBLE|212140572672|77.970001|19.57
PFEUS|PFIZER INC|204834242560|31.879999|14.0878

 

Sample Output:
--------------
 

Lookup for Code IBMUS
code: IBMUS description: International Business Machines last price: 182.95 market cap: 198657.057 Million  P/E: 11.180
Lookup for Code AAPLUS
code: AAPLUS description: Apple Inc last price: 537.370 market cap: 479328.136 Million  P/E: 13.320
Lookup for Code AALLN
code: AALLN description: ANGLO AMERICAN PLC last price: 1589.000 market cap: 22153.061 Million  P/E: 0.100
Lookup for Code 30HK
code: 30HK description: ABC COMMUNICATIONS HLDGS LTD last price: 0.158 market cap: 261.545 Million  P/E: 0.110
All codes:
1398HK
30HK
5HK
857HK
941HK
AALLN
AAPLUS
CVXUS
GEUS
IBMUS
JNJUS
JPMUS
MSFTUS
PFEUS
PGUS
WMTUS
XOMUS
Lowest P/E is 0.100 from code AALLN
The following have P/E between 6.000 and 15.000
code: 5HK description: HSBC HLDGS PLC last price: 84.850 market cap: 1597769.646 Million  P/E: 12.240
code: 857HK description: PETROCHINA CO-H last price: 7.950 market cap: 1744803.856 Million  P/E: 9.560
code: 941HK description: CHINA MOBILE last price: 73.700 market cap: 1482285.122 Million  P/E: 8.910
code: AAPLUS description: Apple Inc last price: 537.370 market cap: 479328.136 Million  P/E: 13.320
code: CVXUS description: CHEVRON CORP last price: 112.710 market cap: 220837.954 Million  P/E: 10.289
code: IBMUS description: International Business Machines last price: 182.950 market cap: 198657.057 Million  P/E: 11.180
code: JPMUS description: JPMORGAN CHASE last price: 58.490 market cap: 217192.350 Million  P/E: 8.420
code: MSFTUS description: MICROSOFT CORP last price: 37.420 market cap: 312081.482 Million  P/E: 13.205
code: PFEUS description: PFIZER INC last price: 31.880 market cap: 204834.243 Million  P/E: 14.088
code: WMTUS description: WAL-MART STORES last price: 75.330 market cap: 241582.768 Million  P/E: 14.400
code: XOMUS description: EXXON MOBIL CORP last price: 94.070 market cap: 413785.620 Million  P/E: 12.830

 


