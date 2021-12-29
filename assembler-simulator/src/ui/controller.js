app.controller('Ctrl', ['$document', '$scope', '$timeout', 'cpu', 'memory', 'global', 'assembler', function ($document, $scope, $timeout, cpu, memory, global,assembler) {
    $scope.memory = memory;
    $scope.global = global;
    $scope.cpu = cpu;
    $scope.error = '';
    $scope.isRunning = false;
    $scope.displayHex = true;
    $scope.displayInstr = true;
    $scope.displayA = false;
    $scope.displayB = false;
    $scope.displayC = false;
    $scope.displayM = true;
    $scope.speeds = [{speed: 1, desc: "1 HZ"},
                     {speed: 4, desc: "4 HZ"},
                     {speed: 8, desc: "8 HZ"},
                     {speed: 16, desc: "16 HZ"},
                     {speed: 32, desc: "32 HZ"},
                     {speed: 64, desc: "64 HZ"},
                     {speed: 128, desc: "128 HZ"},
                     {speed: 256, desc: "256 HZ"}];
    $scope.speed = 16;
    $scope.outputStartIndex = 2536;
    $scope.memoryBlockLength = 256;

    $scope.code = "GOTO start\n\nDB \"Hello World!\"\nDB 0\n\nstart:\nLIL 0x0\nLIH 0x0\nMBA\n\nLIL 0x7\nLIH 0xE\nMMA\nSTA\n\nLIL 0x6\nLIH 0xE\nMMA\nLIL 0x4\nLIH 0x0\nSTA\n\nloop:\nLIL 0x7\nLIH 0xE\nMMA\nLDA\nSEC\nADL\nADH\nMAC\nSTA\n\nLIL 0x6\nLIH 0xE\nMMA\nLDA\nMMA\nLDA\nCLC\nADL\nADH\nLIL 0x7\nLIH 0xE\nMMA\nLDA\nMMA\nMAC\nSTA\n\nLIL 0x6\nLIH 0xE\nMMA\nLDA\nSEC\nADL\nADH\nMAC\nSTA\n\nMMA\nLDA\nCLC\nADL\nADH\n\nLIX loop\nMMA\nJNE\n\nHLT";

    $scope.reset = function () {
        cpu.reset();
        memory.reset();
        $scope.error = '';
        $scope.selectedLine = -1;
    };

    $scope.executeStep = function () {
        if (!$scope.checkPrgrmLoaded()) {
            $scope.assemble();
        }

        try {
            // Execute
            var res = cpu.step();

            // Mark in code
            if (cpu.ip in $scope.mapping) {
                $scope.selectedLine = $scope.mapping[cpu.ip];
            }

            return res;
        } catch (e) {
            $scope.error = e;
            return false;
        }
    };

    var runner;
    $scope.run = function () {
        if (!$scope.checkPrgrmLoaded()) {
            $scope.assemble();
        }

        $scope.isRunning = true;
        runner = $timeout(function () {
            if ($scope.executeStep() === true) {
                $scope.run();
            } else {
                $scope.isRunning = false;
            }
        }, 1000 / $scope.speed);
    };

    $scope.stop = function () {
        $timeout.cancel(runner);
        $scope.isRunning = false;
    };

    $scope.checkPrgrmLoaded = function () {
        for (var i = 0, l = memory.data.length; i < l; i++) {
            if (memory.data[i] !== 0) {
                return true;
            }
        }

        return false;
    };

    $scope.getChar = function (value) {
        var text = String.fromCharCode(value);

        if (text.trim() === '') {
            return '\u00A0\u00A0';
        } else {
            return text;
        }
    };

    $scope.assemble = function () {
        try {
            $scope.reset();

            var assembly = assembler.go($scope.code);
            $scope.mapping = assembly.mapping;
            var binary = assembly.code;
            $scope.labels = assembly.labels;
            $scope.labels_ix = assembly.labels_ix;

            if (binary.length > memory.data.length)
                throw "Binary code does not fit into the memory. Max " + memory.data.length + " bytes are allowed";

            for (var i = 0, l = binary.length; i < l; i++) {
                memory.data[i] = binary[i];
            }
        } catch (e) {
            if (e.line !== undefined) {
                $scope.error = e.line + " | " + e.error;
                $scope.selectedLine = e.line;
            } else {
                $scope.error = e.error;
            }
        }
    };

    $scope.jumpToLine = function (index) {
        $document[0].getElementById('sourceCode').scrollIntoView();
        $scope.selectedLine = $scope.mapping[index];
    };


    $scope.isInstruction = function (index) {
        return $scope.mapping !== undefined &&
            $scope.mapping[index] !== undefined &&
            $scope.displayInstr;
    };

    $scope.getMemoryCellCss = function (index) {
        if (index >= $scope.outputStartIndex) {
            return 'output-bg';
        } else if ($scope.isInstruction(index)) {
            return 'instr-bg';
        } else if (index > cpu.sp && index <= cpu.maxSP) {
            return 'stack-bg';
        } else {
            return '';
        }
    };

    $scope.getMemoryInnerCellCss = function (index) {
        if (index === cpu.pc) {
            return 'marker marker-pc';
        } else if (index === cpu.sp) {
            return 'marker marker-sp';
        } else if (index === cpu.gpr[0] && $scope.displayA) {
            return 'marker marker-a';
        } else if (index === cpu.gpr[1] && $scope.displayB) {
            return 'marker marker-b';
        } else if (index === cpu.gpr[2] && $scope.displayC) {
            return 'marker marker-c';
        } else if (index === cpu.gpr[3] && $scope.displayM) {
            return 'marker marker-m';
        } else {
            return '';
        }
    };
}]);
