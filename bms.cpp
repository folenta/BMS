#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <limits>
#include <algorithm>
#include <bitset>

using namespace std;

// Funkcia reprezentuje RSC koder, na zaklade vstupu a aktualneho stavu vrati vystup a prejde do noveho stavu podla trellis diagramu
string rscEncoder(string output, char input, int *state) {
    switch(*state) {
        case 0:
            if(input == '0') {
                *state = 0;
                output += '0';
            }

            else {
                *state = 2;
                output += '1';
            }

            break;

        case 1:
            if(input == '0') {
                *state = 2;
                output += '1';
            }

            else {
                *state = 0;
                output += '0';
            }

            break;

        case 2:
            if(input == '0') {
                *state = 1;
                output += '0';
            }

            else {
                *state = 3;
                output += '1';
            }

            break;

        case 3:
            if(input == '0') {
                *state = 3;
                output += '1';
            }

            else {
                *state = 1;
                output += '0';
            }

            break;
    }

    return output;
}

// Funkcia prehodi jednotlive znaky v stringu podla vstupneho interleaveru
string interleaveString(string input, string interleaver) {
    string interleavedString = input;

    for (unsigned int i = 0; i < input.size(); i++) {
        int position = interleaver[2*i] - '0';
        interleavedString[i] = input[position];
    }

    return interleavedString;
}

// Funkcia vykonava cinnost turbo kodera
void turboEncoder(string input, string interleaver) {
    string output;
    int registerLength = 2;
    string interleaverOutput = interleaveString(input, interleaver);

    int rscState1 = 0;
    int rscState2 = 0;

    for (unsigned int i = 0; i < input.size(); i++) {
        output += input[i];

        output = rscEncoder(output, input[i], &rscState1);
        output = rscEncoder(output, interleaverOutput[i], &rscState2);
    }

    string binaryRscState1 = bitset<2>(rscState1).to_string();
    string binaryRscState2 = bitset<2>(rscState2).to_string();

    for (int i = registerLength - 1; i >= 0; i--) {
        output += binaryRscState1[i];
        output += binaryRscState1[i];
        output += binaryRscState2[i];
    }

    cout << output << endl;
}

// Funkcia prevedie vstup na postupnost cisel
vector<float> convertInput(string input) {
    stringstream inputStream(input);
    vector<float> inputNumbers;

    while (inputStream.good()) {
        string number;
        getline(inputStream, number, ',');
        inputNumbers.push_back(stof(number));
    }

    return inputNumbers;
}

// Funkcia inicializuje metriky doprednych a spatnych ciest
vector<vector<float>> initializeForwardBackwardPath(int outputLength){

    // Metrika bude reprezentovana ako matica (dlzka vystupu + 1) x (pocet moznych stavov)
    vector<vector<float>> forward(outputLength + 1, vector<float>(4, 0.0));

    // Priradene hodnoty podla navodu
    for (int i = 0; i < outputLength + 1; i++) {
        for (unsigned j = 0; j < 4; j++) {
            if (j == 0)
                forward[i][j] = 0.0;
            else
                forward[i][j] = -numeric_limits<float>::infinity();
        }
    }

    return forward;
}

// Funkcia inicializuje metriku vetvy
vector<vector<vector<float>>> initializeBranch(int outputLength) {

    // Metrika bude reprezentovana ako matica (dlzka vystupu + 1) x (pocet moznych stavov) x (pocet moznych stavov)
    vector<vector<vector<float>>> branch(outputLength + 1, vector<vector<float>>(4, vector<float>(4, 0.0)));

    return branch;
}

// Funkcia ziska kazdy n-ty prvok z postupnosti
vector<float> getEveryNth(vector<float> inputNumbers, int n) {
    vector<float> y;

    for(int i = 0; i < inputNumbers.size(); i++) {
        if (i % 3 == n)
            y.push_back(inputNumbers[i]);
    }

    return y;
}

// Funkcia vrati hodnotu -1/1 na zaklade indexu v "nextStates"
int getIn(int index) {
    int inp;

    if(index % 2 == 0)
        inp = -1;

    else
        inp = 1;

    return inp;
}

// Funkcia vrati hodnotu -1/1 na zaklade indexu v "nextStatesOutput"
int getOut(int index, int output[]) {
    if (output[index] == 0)
        return -1;

    return 1;
}

// Funkcia prehodi jednotlive pozicie znakov v postupnosti podla vstupneho interleaveru
vector<float> interleaveVector(vector<float> vec, string interleaver, int outputLength) {
    vector<float> interleavedVector(outputLength, 0);

    for (int i = 0; i < outputLength - 2; i++) {
        int position = interleaver[2*i] - '0';
        interleavedVector[i] = vec[position];
    }

    return interleavedVector;
}

// Funkcia prehodi jednotlive pozicie znakov v postupnosti do povodneho poradia
vector<float> deinterleaveVector(vector<float> vec, string interleaver, int outputLength) {
    vector<float> interleavedVector(outputLength, 0);

    for (int i = 0; i < outputLength - 2; i++) {
        int position = interleaver[2*i] - '0';
        interleavedVector[position] = vec[i];
    }

    return interleavedVector;
}

// Funkcia realizuje vypocet metriky vetvy
vector<vector<vector<float>>> computeBranchMetrics(vector<vector<vector<float>>> branchMetrics, int outputLength, int possibleTransitions,
        int nextStates[], int nextStatesOutput[], vector<float> y, vector<float> yp, vector<float> La) {

    for(int k = 0; k < outputLength; k++) {
        for(int i = 0; i < possibleTransitions; i++) {
            int state = i / 2;
            int nextState = nextStates[i];
            int inp = getIn(i);
            int out = getOut(i, nextStatesOutput);

            branchMetrics[k][state][nextState] = inp * y[k] + out * yp[k] + inp * La[k];
        }
    }

    return branchMetrics;
}

// Funkcia realizuje vypocet metriky doprednej cesty
vector<vector<float>> computeForwardPathMetrics(vector<vector<float>> forwardPathMetrics, vector<vector<vector<float>>> branchMetrics,
        int outputLength, int pastStates[]) {

    for(int k = 1; k < outputLength + 1; k++) {
        for (unsigned state = 0; state < 4; state++) {

            int pastState1 = pastStates[2 * state];
            int pastState2 = pastStates[2 * state + 1];

            float forwardPathMetricsValue1 = forwardPathMetrics[k - 1][pastState1];
            float forwardPathMetricsValue2 = forwardPathMetrics[k - 1][pastState2];

            float branchMetricsValue1 = branchMetrics[k - 1][pastState1][state];
            float branchMetricsValue2 = branchMetrics[k - 1][pastState2][state];

            float value1 = forwardPathMetricsValue1 + branchMetricsValue1;
            float value2 = forwardPathMetricsValue2 + branchMetricsValue2;

            if (value1 > value2)
                forwardPathMetrics[k][state] = value1;
            else
                forwardPathMetrics[k][state] = value2;
        }
    }

    return forwardPathMetrics;
}

// Funkcia realizuje vypocet metriky spatnej cesty
vector<vector<float>> computeBackwardPathMetrics(vector<vector<float>> backwardPathMetrics, vector<vector<vector<float>>> branchMetrics,
        int outputLength, int nextStates[]) {

    for(int k = 1; k < outputLength + 1; k++) {
        for (unsigned int state = 0; state < 4; state++) {

            int nextState1 = nextStates[2*state];
            int nextState2 = nextStates[2*state+1];

            float backwardPathMetricsValue1 = backwardPathMetrics[k-1][nextState1];
            float backwardPathMetricsValue2 = backwardPathMetrics[k-1][nextState2];

            int r = outputLength - k;

            float branchMetricsValue1 = branchMetrics[r][state][nextState1];
            float branchMetricsValue2 = branchMetrics[r][state][nextState2];

            float value1 = backwardPathMetricsValue1 + branchMetricsValue1;
            float value2 = backwardPathMetricsValue2 + branchMetricsValue2;

            if (value1 > value2)
                backwardPathMetrics[k][state] = value1;
            else
                backwardPathMetrics[k][state] = value2;
        }
    }

    return backwardPathMetrics;
}

// Funkcia realizuje vypocet LLR
vector<float> computeLLR(vector<vector<vector<float>>> branchMetrics, vector<vector<float>> forwardPathMetrics, vector<vector<float>> backwardPathMetrics,
        int outputLength, int possibleTransitions, int nextStates[], int nextStatesOutput[]) {

    vector<float> LLR;

    for(int k = 0; k < outputLength; k++) {
        int r = outputLength - k - 1;

        vector<float> inputWas1;
        vector<float> inputWas0;

        for (int i = 0; i < possibleTransitions; i++) {
            int state = i / 2;
            int nextState = nextStates[i];
            int inp = getIn(i);

            float forwardM = forwardPathMetrics[k][state];
            float branchM = branchMetrics[k][state][nextState];
            float backwardM = backwardPathMetrics[r][nextState];

            float value = forwardM + branchM + backwardM;

            if (inp < 0)
                inputWas0.push_back(value);
            else
                inputWas1.push_back(value);

        }

        float LLRk =
                *max_element(inputWas1.begin(), inputWas1.end()) - *max_element(inputWas0.begin(), inputWas0.end());
        LLR.push_back(LLRk);

    }

    return LLR;
}

// Funkcia vykonava cinnost SISO dekodera
vector<float> SISODecoder(vector<float> inputNumbers, int outputLength, int pastStates[], int nextStates[], int nextStatesOutput[],
        int possibleTransitions, vector<float> y, vector<float> yp, vector<float> La) {

    // Inicializacia metrik
    vector<vector<float>> forwardPathMetrics = initializeForwardBackwardPath(outputLength);
    vector<vector<float>> backwardPathMetrics = initializeForwardBackwardPath(outputLength);
    vector<vector<vector<float>>> branchMetrics = initializeBranch(outputLength);

    vector<float> LLR;

    // Vypocet metrik
    branchMetrics = computeBranchMetrics(branchMetrics, outputLength, possibleTransitions, nextStates, nextStatesOutput, y, yp, La);
    forwardPathMetrics = computeForwardPathMetrics(forwardPathMetrics, branchMetrics, outputLength, pastStates);
    backwardPathMetrics = computeBackwardPathMetrics(backwardPathMetrics, branchMetrics, outputLength, nextStates);

    // Vypocet LLR na zaklade metrik
    LLR = computeLLR(branchMetrics, forwardPathMetrics, backwardPathMetrics, outputLength, possibleTransitions, nextStates, nextStatesOutput);

    return LLR;

}

// Funckia vytlaci vystup
void printOutput(int outputLength, int registerLength, vector<float> LLR) {
    for (int k = 0; k < outputLength - registerLength; k++){
        if (LLR[k] < 0)
            cout << 0;
        else
            cout << 1;
    }

    cout << endl;
}

// Funkcia realizuje cinnost turbo dekodera
void turboDecoder(string input, string interleaver) {

    vector<float> inputNumbers = convertInput(input);
    int outputLength = inputNumbers.size() / 3;
    int registerLength = 2;

    int pastStates [8] = {0, 1, 2, 3, 0, 1, 2, 3};
    int nextStates [8] = {0, 2, 2, 0, 1, 3, 3, 1};
    int nextStatesOutput [8] = {0, 1, 1, 0, 0, 1, 1, 0};

    int possibleTransitions = sizeof(nextStates)/sizeof(nextStates[0]);

    vector<float> y = getEveryNth(inputNumbers, 0);
    vector<float> yp1 = getEveryNth(inputNumbers, 1);
    vector<float> yp2 = getEveryNth(inputNumbers, 2);
    vector<float> initialLa(outputLength, 0);

    vector<float> LLR1 = SISODecoder(inputNumbers, outputLength, pastStates, nextStates, nextStatesOutput, possibleTransitions, y, yp1, initialLa);
    vector<float> interleavedLLR1 = interleaveVector(LLR1, interleaver, outputLength);

    vector<float> LLR2 = SISODecoder(inputNumbers, outputLength, pastStates, nextStates, nextStatesOutput, possibleTransitions, y, yp2, interleavedLLR1);
    LLR2 = deinterleaveVector(LLR2, interleaver, outputLength);

    printOutput(outputLength, registerLength, LLR2);
}

int main(int argc, char *argv[]) {

    string mode = argv[1];
    string interleaver = argv[3];
    string input;

    cin >> input;

    if (mode == "-e")
        turboEncoder(input, interleaver);

    if (mode == "-d")
        turboDecoder(input, interleaver);

    return 0;
}
