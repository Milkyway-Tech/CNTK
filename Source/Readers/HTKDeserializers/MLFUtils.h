//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
//

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/noncopyable.hpp>
#include "Indexer.h"

namespace Microsoft { namespace MSR { namespace CNTK {

    // Representation of a state list table.
    // Whole table is preserved in memory, because it is never more than thousands states.
    class StateTable : boost::noncopyable
    {
    public:
        void ReadStateList(const std::wstring& stateListPath);

        const std::vector<bool>& SilStateMask() const
        {
            return m_silStateMask;
        }

        const std::unordered_map<std::string, size_t>& States() const
        {
            return m_stateTable;
        }

    private:
        bool IsSilState(const std::string& stateName) const
        {
            return stateName.size() > 3 && !strncmp(stateName.c_str(), "sil", 3);
        }

        static std::vector<boost::iterator_range<char*>> ReadLines(const std::wstring& path, std::vector<char>& buffer);

        std::vector<bool> m_silStateMask; // [state index] => true if is sil state (cached)
        std::unordered_map<std::string, size_t> m_stateTable; // for state <=> index
    };

    typedef std::shared_ptr<StateTable> StateTablePtr;
    typedef unsigned short ClassIdType;

    // Representation an MLF range.
    class MLFFrameRange
    {
        static const double htkTimeToFrame;

        ClassIdType m_classId;     // numeric state id
        uint32_t m_firstFrame;     // start frame
        uint32_t m_numFrames;      // number of frames

    public:
        // Parses format with original HTK state align MLF format and state list and builds an MLFFrameRange.
        void Build(const vector<boost::iterator_range<char*>>& tokens, const unordered_map<std::string, size_t>& stateTable);

        ClassIdType ClassId() const { return m_classId;    }
        uint32_t FirstFrame() const { return m_firstFrame; }
        uint32_t NumFrames()  const { return m_numFrames;  }

        // Note: preserving logic of the old speech reader.
        // Parse the time range.
        // There are two formats:
        //  - original HTK
        //  - Dong's hacked format: ts te senonename senoneid
        static std::pair<size_t, size_t> ParseFrameRange(const std::vector<boost::iterator_range<char*>>& tokens);

    private:
        void VerifyAndSaveRange(const std::pair<size_t, size_t>& frameRange, size_t uid);
    };

    // Utility class for parsing an mlf utterance.
    class MLFUtteranceParser
    {
        const StateTablePtr m_states;

    public:
        MLFUtteranceParser(const StateTablePtr& states) : m_states(states)
        {}

        bool Parse(const SequenceDescriptor& utterance, const boost::iterator_range<char*>& utteranceData, std::vector<MLFFrameRange>& result);
    };

}}} // namespace