#include <fstream>
#include <iostream>
#include <sstream> 
#include <vector>
#include <algorithm>
#include <iterator>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <set>
#include <thread>
#include <cmath>

// #include "convert_date.h"

//g++ -O2 -std=c++17 -pthread -o reduceOptions_Parallel.o reduceOptions_Parallel.cpp

typedef std::vector<std::string> stringvec;

//0<A<a
struct alphanumcompare
{
    bool operator()(const std::string &lhs, const std::string &rhs)
    {
        return lhs < rhs;
    }
};


struct Option
{
    // bool type; //put is 0, call is 1
    std::string symbol;
    std::string Bid;
    std::string Ask;
    std::string Bidsize;
    std::string Asksize;
    // double highPrice;
    // double lowPrice;
    std::string Volatility;
    std::string delta;
    std::string gamma;
    std::string theta;
    std::string vega;
    std::string rho;
    std::string openInterest;
    std::string theoreticalOptionValue;
    std::string theoreticalVolatility;
    // double strikePrice;
    // unsigned long long int expirationDate;
    // std::string multiplier;
    // std::string strikePriceS;
    // std::string expirationDateS;
    // std::string typeS;
};

std::string path = "REDACTED/TDOptionsData/Temporary_Data";
std::string dataholdingfolder = "REDACTED/TDOptionsData";

const int threads {4};
const bool useExtended = false;

void processData_1Thread(stringvec &v);
inline bool fileExists(const std::string &name);
int write2File(Option &writeoption, std::string &timestamp);
void read_directory(const std::string &name, stringvec &v);
std::array<std::vector<std::string>, threads> &assignJobs(const std::vector<std::string> &filelist);

//files that would have been included in convert_date.cpp
// std::string encodedate(const std::string &stringtoinclude);
//std::array<char, 5> convert10tobase121(int N);
//std::array<char, 5> convert10tobase93(int N);
// inline char digit2lookuplong(char &digitval);
// inline char digit2lookupshort(char &digitval);

int main()
{
    std::cout << "Reduce Options Multithreaded" << std::endl;

    stringvec v;
    read_directory(path, v);

    //0<A<a
    alphanumcompare myComparatorObject;
    std::sort(v.begin(), v.end(), myComparatorObject);
    //check to make sure it is lowest first, and not highest first

    auto tickerjobs = assignJobs(v);

    std::array<std::thread, threads> threadlist;

    for (int i = 0; i < threads; i++)
    {
        threadlist.at(i) = std::thread(processData_1Thread, std::ref((stringvec &)tickerjobs.at(i)));
    }

    for (int i = 0; i < threads; i++)
    {
        threadlist.at(i).join();
    }

    std::cout << "Done" << std::endl;
    system(("exec rm -rf " + path).c_str());
    system(("mkdir " + path).c_str());

    return 0;
}

void processData_1Thread(stringvec &v)
{
    for (auto &elem : v)
    {
        if (elem.at(0) == '.')
            continue;

        std::ifstream inFile;
        inFile.open(path + '/' + elem);
        std::stringstream strStream;
        strStream << inFile.rdbuf(); //read the file
        inFile.close();
        std::string str = strStream.str(); //str holds the content of the file

        bool dataon = false;
        bool fieldon = false;
        char strbuff[50]{};
        char *buffptr = strbuff;

        Option option2write;

        for (char const &c : str)
        {
            if (c == 0)
            {
                break; //do not error on null string
            }
            if (c == '{' && !dataon)
            {
                dataon = true;
                buffptr = strbuff;
                continue;
            }

            if (dataon)
            {   
                if (c == ',' || c == '}')
                {
                    std::string temp = strbuff;
                    temp.erase(remove(temp.begin(), temp.end(), ' '), temp.end());
                    std::size_t splitter = temp.find(':');
                    std::string seriesname = temp.substr(1, splitter - 2);
                    std::string datum = temp.substr(splitter + 1);
                    
                    // std::cout<<"Series: "<<seriesname<<std::endl;
                    // std::cout<<"Datum: "<<datum<<std::endl;

                    // if (seriesname == "putCall")
                    // {
                    //     if (datum == "PUT")
                    //     {
                    //         option2write.type = false;
                    //     }
                    //     else
                    //     {
                    //         option2write.type = true;
                    //     }
                    //     option2write.typeS = datum;
                    // }
                    // else
                    if (seriesname == "symbol")
                    {
                        option2write.symbol = datum;
                    }
                    else if (seriesname == "bidAskSize")
                    {
                        option2write.Bidsize = datum.substr(1, datum.find('X') - 1);

                        option2write.Asksize = datum.substr(datum.find('X')+1, datum.rfind('\'')-(datum.find('X')+1));
                    }
                    else if (seriesname == "bid")
                    {
                        option2write.Bid = datum;
                    }
                    else if (seriesname == "ask")
                    {
                        option2write.Ask = datum;
                    }
                    else if (seriesname == "volatility")
                    {
                        option2write.Volatility = datum;
                    }
                    else if (seriesname == "delta")
                    {
                        option2write.delta = datum;
                    }
                    else if (seriesname == "gamma")
                    {
                        option2write.gamma = datum;
                    }
                    else if (seriesname == "theta")
                    {
                        option2write.theta = datum;
                    }
                    else if (seriesname == "vega")
                    {
                        option2write.vega = datum;
                    }
                    else if (seriesname == "rho")
                    {
                        option2write.rho = datum;
                    }
                    else if (seriesname == "openInterest")
                    {
                        option2write.openInterest = datum;
                    }
                    else if (seriesname == "theoreticalOptionValue")
                    {
                        option2write.theoreticalOptionValue = datum;
                    }
                    else if (seriesname == "theoreticalVolatility")
                    {
                        option2write.theoreticalVolatility = datum;
                    }
                    // else if (seriesname == "strikePrice")
                    // {
                    //     option2write.strikePriceS = datum;
                    // }
                    // else if (seriesname == "expirationDate")
                    // {
                    //     option2write.expirationDateS = datum;
                    // }
                    // //Note, that it was risky not to include multiplier, but apparently 100 is the standard multiplier for the US stocks which all these are.  A sanity check can also be there, but the data here is more important.
                    // else if (seriesname == "multiplier")
                    // {
                    //     option2write.multiplier = datum;
                    // }

                    buffptr = strbuff;                                //reset the writing pointer
                    std::fill(strbuff, strbuff + sizeof(strbuff), 0); //reseting buffer

                    if (c == '}')
                    {
                        dataon = false;

                        std::string timestamp = elem.substr(0, elem.find('.')); 
                        //write data to file                        
                        if(write2File(option2write, timestamp)==0){std::cout<<"Writing to File Failed!"<<std::endl;}
                    }
                    continue;
                }
                //record the value from the beginning until the comma, then you can parse that individual element
                *buffptr = c;
                buffptr++;
            }
        }
    }
}

inline bool fileExists(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

void read_directory(const std::string &name, stringvec &v)
{
    DIR *dirp = opendir(name.c_str());
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL)
    {
        v.push_back(dp->d_name);
    }
    closedir(dirp);
}

std::array<std::vector<std::string>, threads> &assignJobs(const std::vector<std::string> &filelist)
{

    static std::set<std::string> uniquetickers;
    static std::vector<std::string> orderedtickers;
    for (const std::string &elem : filelist)
    {
        if (elem.at(0) != '.')
        {
            auto start = elem.find('_') + 1;
            auto end = elem.rfind('_') - 1;
            auto appendelem = elem.substr(start, end - start + 1);
            if (uniquetickers.count(appendelem) == 0)
            {
                //item is not in set
                uniquetickers.insert(appendelem);
                orderedtickers.push_back(appendelem);
            }
        }
    }

    int counter = 0;
    static std::array<std::vector<std::string>, threads> assignmentlist;
    for (int i = 0; i < std::ceil(orderedtickers.size() / 2.0); i++) //iterate over pairs of elements
    {
        for (const std::string &elem : filelist) //iterate over all files
        {
            if (elem.find('_' + orderedtickers.at(i) + '_') != elem.npos || elem.find('_' + orderedtickers.at(orderedtickers.size() - 1 - i) + '_') != elem.npos)
            {
                assignmentlist.at(counter).push_back(elem);
            }
        }
        counter++;
        if (counter > threads - 1)
            counter = 0;
    }

    return assignmentlist;
}

int write2File(Option &writeoption, std::string &timestamp)
{
    std::string filepath = dataholdingfolder + '/' + writeoption.symbol.substr(1, writeoption.symbol.find('_') - 1) + "/" + writeoption.symbol.substr(1, writeoption.symbol.length() - 2) + ".csv";

    bool fileexists = fileExists(filepath);
    //looking back on this, it seems this is checking if a file exists and then tries to make a folder.  it should check for the folder.
    if (!fileexists)
    {
        //file does not exist
        mkdir((dataholdingfolder + '/' + writeoption.symbol.substr(1, writeoption.symbol.find('_') - 1)).c_str(), 0777);
        //mkdir can fail, but it does nothing if it does, just returns -1
    }

    //note the folder needs to exist for this to create a file
    std::ofstream file2write;
    file2write.open(filepath, std::ios::out | std::ios::app); //open a filestream for new stream (output, append)
    if (file2write.fail())
        {
        return 1;
        std::cout<<"opening file failed"<<std::endl;
        }

    if (!fileexists)
    {
        //write the header:
        file2write << "TS,"
                   << "BidS,"
                   << "Bid,"
                   << "Ask,"
                   << "AskS,"
                   << "V,"
                   << "OI,"
                   << "d,"
                   << "g,"
                   << "t,"
                   << "v,"
                   << "r,"
                   << "tov,"
                   << "tV"
                   << "\n";
    }
    //write the datapoint:
    // std::cout<<encodedate(timestamp)<<std::endl;
    file2write << timestamp << ',' //timestamp //encodedate(timestamp)
               << writeoption.Bidsize << ','
               << writeoption.Bid << ','
               << writeoption.Ask << ','
               << writeoption.Asksize << ','
               << writeoption.Volatility << ','
               << writeoption.openInterest << ','
               << writeoption.delta << ','
               << writeoption.gamma << ','
               << writeoption.theta << ','
               << writeoption.vega << ','
               << writeoption.rho << ','
               << writeoption.theoreticalOptionValue << ','
               << writeoption.theoreticalVolatility
               << "\n";

    file2write.close();
    return 0; 
}

//these would have been included in convert_date.cpp had date conversion been used.

// std::string encodedate(const std::string &stringtoinclude)
// {
//     std::string::size_type sz;
//     int timestamp = std::stoi(stringtoinclude.substr(0, stringtoinclude.find('.')), &sz);
//     std::array<char, 5> temparr;
//     if (useExtended)
//     {
//         temparr = convert10tobase121(timestamp);
//     }
//     else
//     {
//         temparr = convert10tobase93(timestamp);
//     }
//
//     std::string returnstr;
//     for (char elem : temparr)
//     {
//         // std::cout <<(int)elem<<std::endl<< (int)digit2lookup(elem)<<std::endl<< digit2lookup(elem) << std::endl;
//         returnstr += digit2lookupshort(elem);
//     }
//     return returnstr;
// }


//Although these two functions could be combined to make a single function, they were left this way for clarity, as the lookup dictionaries at the end of the file are designed specifically for base 121 and 93 ASCII characters

// using std::pow;
// std::array<char, 5> convert10tobase121(int N)
// {
//     std::array<char, 5> outarr{0};
//     // for (char elem:outarr)
//     // {
//     //         std::cout<<(int)elem<<std::endl;
//     // }
//     // std::cout<<std::endl;
//     for (int i = 4; i >= 0; i--)
//     {
//         if (N >= pow(121, i))
//         {
//             outarr[4 - i] = N / static_cast<int>(pow(121, i));
//             N = N - outarr[i] * pow(121, i);
//         }
//
//         if (N == 0)
//         {
//             break;
//         }
//     }
//     return outarr;
// }
//
// std::array<char, 5> convert10tobase93(int N)
// {
//     std::array<char, 5> outarr{0};
//     for (int i = 4; i >= 0; i--)
//     {
//         if (N >= pow(93, i))
//         {
//             outarr[4 - i] = N / static_cast<int>(pow(93, i));
//             N = N - outarr[i] * pow(93, i);
//         }
//
//         if (N == 0)
//         {
//             break;
//         }
//     }
//     return outarr;
// }



// inline char digit2lookupshort(char &digitval)
// { //skipping whitespace/paragraphing symbols that throw off pandas (0-32), comma(44), del(127) (doesn't seem to do anything/show.)

//     //93 unique elements can encode 220.4 years with 5 digits.  This works.
//     switch (digitval)
//     {
//     case 0:
//         return 33;
//         break;
//     case 1:
//         return 34;
//         break;
//     case 2:
//         return 35;
//         break;
//     case 3:
//         return 36;
//         break;
//     case 4:
//         return 37;
//         break;
//     case 5:
//         return 38;
//         break;
//     case 6:
//         return 39;
//         break;
//     case 7:
//         return 40;
//         break;
//     case 8:
//         return 41;
//         break;
//     case 9:
//         return 42;
//         break;
//     case 10:
//         return 43;
//         break;

//     case 11:
//         return 45;
//         break;
//     case 12:
//         return 46;
//         break;
//     case 13:
//         return 47;
//         break;
//     case 14:
//         return 48;
//         break;
//     case 15:
//         return 49;
//         break;
//     case 16:
//         return 50;
//         break;
//     case 17:
//         return 51;
//         break;
//     case 18:
//         return 52;
//         break;
//     case 19:
//         return 53;
//         break;
//     case 20:
//         return 54;
//         break;
//     case 21:
//         return 55;
//         break;
//     case 22:
//         return 56;
//         break;
//     case 23:
//         return 57;
//         break;
//     case 24:
//         return 58;
//         break;
//     case 25:
//         return 59;
//         break;
//     case 26:
//         return 60;
//         break;
//     case 27:
//         return 61;
//         break;
//     case 28:
//         return 62;
//         break;
//     case 29:
//         return 63;
//         break;
//     case 30:
//         return 64;
//         break;
//     case 31:
//         return 65;
//         break;
//     case 32:
//         return 66;
//         break;
//     case 33:
//         return 67;
//         break;
//     case 34:
//         return 68;
//         break;
//     case 35:
//         return 69;
//         break;
//     case 36:
//         return 70;
//         break;
//     case 37:
//         return 71;
//         break;
//     case 38:
//         return 72;
//         break;
//     case 39:
//         return 73;
//         break;
//     case 40:
//         return 74;
//         break;
//     case 41:
//         return 75;
//         break;
//     case 42:
//         return 76;
//         break;
//     case 43:
//         return 77;
//         break;
//     case 44:
//         return 78;
//         break;
//     case 45:
//         return 79;
//         break;
//     case 46:
//         return 80;
//         break;
//     case 47:
//         return 81;
//         break;
//     case 48:
//         return 82;
//         break;
//     case 49:
//         return 83;
//         break;
//     case 50:
//         return 84;
//         break;
//     case 51:
//         return 85;
//         break;
//     case 52:
//         return 86;
//         break;
//     case 53:
//         return 87;
//         break;
//     case 54:
//         return 88;
//         break;
//     case 55:
//         return 89;
//         break;
//     case 56:
//         return 90;
//         break;
//     case 57:
//         return 91;
//         break;
//     case 58:
//         return 92;
//         break;
//     case 59:
//         return 93;
//         break;
//     case 60:
//         return 94;
//         break;
//     case 61:
//         return 95;
//         break;
//     case 62:
//         return 96;
//         break;
//     case 63:
//         return 97;
//         break;
//     case 64:
//         return 98;
//         break;
//     case 65:
//         return 99;
//         break;
//     case 66:
//         return 100;
//         break;
//     case 67:
//         return 101;
//         break;
//     case 68:
//         return 102;
//         break;
//     case 69:
//         return 103;
//         break;
//     case 70:
//         return 104;
//         break;
//     case 71:
//         return 105;
//         break;
//     case 72:
//         return 106;
//         break;
//     case 73:
//         return 107;
//         break;
//     case 74:
//         return 108;
//         break;
//     case 75:
//         return 109;
//         break;
//     case 76:
//         return 110;
//         break;
//     case 77:
//         return 111;
//         break;
//     case 78:
//         return 112;
//         break;
//     case 79:
//         return 113;
//         break;
//     case 80:
//         return 114;
//         break;
//     case 81:
//         return 115;
//         break;
//     case 82:
//         return 116;
//         break;
//     case 83:
//         return 117;
//         break;
//     case 84:
//         return 118;
//         break;
//     case 85:
//         return 119;
//         break;
//     case 86:
//         return 120;
//         break;
//     case 87:
//         return 121;
//         break;
//     case 88:
//         return 122;
//         break;
//     case 89:
//         return 123;
//         break;
//     case 90:
//         return 124;
//         break;
//     case 91:
//         return 125;
//         break;
//     case 92:
//         return 126;
//         break;
//     }
// }

// inline char digit2lookuplong(char &digitval)
// {
//     //skipping null (0), tab (9) (messes up the orientation, but could still be used),
//     //linefeed/newline (10), carriage return (13) (could be used),
//     //space (32), comma(44), del(127) (doesn't seem to do anything/show.)

//     //121 unique elements can encode 821.9 years with 5 digits.  This works.
//     switch (digitval)
//     {
//     case 0:
//         return 1;
//         break;
//     case 1:
//         return 2;
//         break;
//     case 2:
//         return 3;
//         break;
//     case 3:
//         return 4;
//         break;
//     case 4:
//         return 5;
//         break;
//     case 5:
//         return 6;
//         break;
//     case 6:
//         return 7;
//         break;
//     case 7:
//         return 8;
//         break;
//     case 8:
//         return 11;
//         break;
//     case 9:
//         return 12;
//         break;
//     case 10:
//         return 14;
//         break;
//     case 11:
//         return 15;
//         break;
//     case 12:
//         return 16;
//         break;
//     case 13:
//         return 17;
//         break;
//     case 14:
//         return 18;
//         break;
//     case 15:
//         return 19;
//         break;
//     case 16:
//         return 20;
//         break;
//     case 17:
//         return 21;
//         break;
//     case 18:
//         return 22;
//         break;
//     case 19:
//         return 23;
//         break;
//     case 20:
//         return 24;
//         break;
//     case 21:
//         return 25;
//         break;
//     case 22:
//         return 26;
//         break;
//     case 23:
//         return 27;
//         break;
//     case 24:
//         return 28;
//         break;
//     case 25:
//         return 29;
//         break;
//     case 26:
//         return 30;
//         break;
//     case 27:
//         return 31;
//         break;
//     case 28:
//         return 33;
//         break;
//     case 29:
//         return 34;
//         break;
//     case 30:
//         return 35;
//         break;
//     case 31:
//         return 36;
//         break;
//     case 32:
//         return 37;
//         break;
//     case 33:
//         return 38;
//         break;
//     case 34:
//         return 39;
//         break;
//     case 35:
//         return 40;
//         break;
//     case 36:
//         return 41;
//         break;
//     case 37:
//         return 42;
//         break;
//     case 38:
//         return 43;
//         break;
//     case 39:
//         return 44;
//         break;
//     case 40:
//         return 45;
//         break;
//     case 41:
//         return 46;
//         break;
//     case 42:
//         return 47;
//         break;
//     case 43:
//         return 48;
//         break;
//     case 44:
//         return 49;
//         break;
//     case 45:
//         return 50;
//         break;
//     case 46:
//         return 51;
//         break;
//     case 47:
//         return 52;
//         break;
//     case 48:
//         return 53;
//         break;
//     case 49:
//         return 54;
//         break;
//     case 50:
//         return 55;
//         break;
//     case 51:
//         return 56;
//         break;
//     case 52:
//         return 57;
//         break;
//     case 53:
//         return 58;
//         break;
//     case 54:
//         return 59;
//         break;
//     case 55:
//         return 60;
//         break;
//     case 56:
//         return 61;
//         break;
//     case 57:
//         return 62;
//         break;
//     case 58:
//         return 63;
//         break;
//     case 59:
//         return 64;
//         break;
//     case 60:
//         return 65;
//         break;
//     case 61:
//         return 66;
//         break;
//     case 62:
//         return 67;
//         break;
//     case 63:
//         return 68;
//         break;
//     case 64:
//         return 69;
//         break;
//     case 65:
//         return 70;
//         break;
//     case 66:
//         return 71;
//         break;
//     case 67:
//         return 72;
//         break;
//     case 68:
//         return 73;
//         break;
//     case 69:
//         return 74;
//         break;
//     case 70:
//         return 75;
//         break;
//     case 71:
//         return 76;
//         break;
//     case 72:
//         return 77;
//         break;
//     case 73:
//         return 78;
//         break;
//     case 74:
//         return 79;
//         break;
//     case 75:
//         return 80;
//         break;
//     case 76:
//         return 81;
//         break;
//     case 77:
//         return 82;
//         break;
//     case 78:
//         return 83;
//         break;
//     case 79:
//         return 84;
//         break;
//     case 80:
//         return 85;
//         break;
//     case 81:
//         return 86;
//         break;
//     case 82:
//         return 87;
//         break;
//     case 83:
//         return 88;
//         break;
//     case 84:
//         return 89;
//         break;
//     case 85:
//         return 90;
//         break;
//     case 86:
//         return 91;
//         break;
//     case 87:
//         return 92;
//         break;
//     case 88:
//         return 93;
//         break;
//     case 89:
//         return 94;
//         break;
//     case 90:
//         return 95;
//         break;
//     case 91:
//         return 96;
//         break;
//     case 92:
//         return 97;
//         break;
//     case 93:
//         return 98;
//         break;
//     case 94:
//         return 99;
//         break;
//     case 95:
//         return 100;
//         break;
//     case 96:
//         return 101;
//         break;
//     case 97:
//         return 102;
//         break;
//     case 98:
//         return 103;
//         break;
//     case 99:
//         return 104;
//         break;
//     case 100:
//         return 105;
//         break;
//     case 101:
//         return 106;
//         break;
//     case 102:
//         return 107;
//         break;
//     case 103:
//         return 108;
//         break;
//     case 104:
//         return 109;
//         break;
//     case 105:
//         return 110;
//         break;
//     case 106:
//         return 111;
//         break;
//     case 107:
//         return 112;
//         break;
//     case 108:
//         return 113;
//         break;
//     case 109:
//         return 114;
//         break;
//     case 110:
//         return 115;
//         break;
//     case 111:
//         return 116;
//         break;
//     case 112:
//         return 117;
//         break;
//     case 113:
//         return 118;
//         break;
//     case 114:
//         return 119;
//         break;
//     case 115:
//         return 120;
//         break;
//     case 116:
//         return 121;
//         break;
//     case 117:
//         return 122;
//         break;
//     case 118:
//         return 123;
//         break;
//     case 119:
//         return 124;
//         break;
//     case 120:
//         return 125;
//         break;
//     }
// }
