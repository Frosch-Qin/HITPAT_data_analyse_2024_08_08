#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>

#include "TFile.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLine.h"


#pragma once

// To find the equally spaced local maxima in a TH1D histogram
// The idea is to first identify some peaks
// Then extract the mode or median of the distance between the peaks
// use the mode or median, to judge if the peaks are equally spaced
// if not, add or remove peaks

struct Peak
{
    double x;  // position in bin number
    double y;  // height in bin content
    int order; // peak order
};

struct JudgePeak
{
    int RemovePeak = 0;
    int AddPeak = 0;
    std::vector<int> RemovePeakIndices; // gap indices i: between peak i and i+1
    std::vector<int> AddPeakIndices;    // gap indices i: between peak i and i+1
};

static void sort_and_reorder(std::vector<Peak> &peaks)
{
    std::sort(peaks.begin(), peaks.end(),
              [](const Peak &a, const Peak &b)
              { return a.x < b.x; });
    for (size_t i = 0; i < peaks.size(); ++i)
        peaks[i].order = int(i) + 1;
}

static std::vector<Peak> find_peaks_in_range(TH1D *hist, int bin_lo, int bin_hi, double threshold_abs)
{
    std::vector<Peak> out;
    const int nb = hist->GetNbinsX();

    bin_lo = std::max(bin_lo, 2);
    bin_hi = std::min(bin_hi, nb - 1);
    if (bin_lo > bin_hi)
        return out;

    int count = 0;
    for (int i = bin_lo; i <= bin_hi; ++i)
    {
        double this_bin = hist->GetBinContent(i);
        double prev_bin = hist->GetBinContent(i - 1);
        double next_bin = hist->GetBinContent(i + 1);

        if (this_bin > threshold_abs && this_bin >= prev_bin && this_bin >= next_bin)
        {
            Peak p;
            p.x = i;
            p.y = this_bin;
            p.order = ++count;
            out.push_back(p);
        }
    }
    sort_and_reorder(out);
    return out;
}

std::vector<Peak> number_of_peaks_above_threshold(TH1D *hist, double threshold_abs)
{
    return find_peaks_in_range(hist, 2, hist->GetNbinsX() - 1, threshold_abs);
}

// ---- robust spacing estimators ----

// median of distances (safe & robust)
double median_distance(std::vector<double> d) // pass-by-value so we can sort
{
    if (d.empty())
        return 0.0;
    std::sort(d.begin(), d.end());
    if (d.size() % 2 == 1)
        return d[d.size() / 2];
    return 0.5 * (d[d.size() / 2 - 1] + d[d.size() / 2]);
}

// majority distance = mode (rounded to nearest bin) -- best for bin-index spacing
double majority_distance_mode_rounded(const std::vector<double> &distances)
{
    if (distances.empty())
        return 0.0;

    std::map<int, int> cnt; // rounded distance -> count
    for (double x : distances)
    {
        int di = (int)std::lround(x);
        if (di > 0)
            cnt[di]++;
    }
    if (cnt.empty())
        return 0.0;

    int best_d = 0;
    int best_c = -1;
    for (const auto &kv : cnt)
    {
        if (kv.second > best_c)
        {
            best_c = kv.second;
            best_d = kv.first;
        }
    }
    return (double)best_d;
}

void judge_peaks(const std::vector<Peak> &peak_positions, JudgePeak &result)
{
    result.RemovePeak = 0;
    result.AddPeak = 0;
    result.RemovePeakIndices.clear();
    result.AddPeakIndices.clear();

    if (peak_positions.size() < 2)
    {
        std::cout << "Not enough peaks to judge.\n";
        return;
    }

    std::vector<double> distances;
    distances.reserve(peak_positions.size() - 1);
    for (size_t i = 1; i < peak_positions.size(); ++i)
        distances.push_back(peak_positions[i].x - peak_positions[i - 1].x);

    // Choose your "typical distance":
    // 1) median (robust)
    // double ref = median_distance(distances);

    // 2) mode/majority (recommended for equally spaced many peaks)
    double ref = majority_distance_mode_rounded(distances);

    if (ref <= 0)
    {
        std::cout << "Cannot determine reference distance.\n";
        return;
    }

    // These factors are tunable:
    const double far_factor = 1.3;   // > 1.3*ref => likely missing peak
    const double close_factor = 0.7; // < 0.7*ref => likely extra peak / too close

    for (size_t i = 0; i < distances.size(); ++i)
    {
        double d = distances[i];
        if (d > far_factor * ref)
        {
            result.AddPeak++;
            result.AddPeakIndices.push_back((int)i);
        }
        else if (d < close_factor * ref)
        {
            result.RemovePeak++;
            result.RemovePeakIndices.push_back((int)i);
        }
    }

    std::cout << "Reference distance (majority/mode) = " << ref
              << ", Add=" << result.AddPeak
              << ", Remove=" << result.RemovePeak << "\n";
}

static bool remove_one_close_pair(std::vector<Peak> &peaks, int remove_index)
{
    if (peaks.size() < 2)
        return false;
    sort_and_reorder(peaks);

    if (remove_index < 0 || remove_index + 1 >= (int)peaks.size())
        return false;

    int i = remove_index;
    size_t idx_remove = (peaks[i].y < peaks[i + 1].y) ? (size_t)i : (size_t)(i + 1);

    std::cout << "Removing close peak at x=" << peaks[idx_remove].x
              << " y=" << peaks[idx_remove].y
              << " (gap index=" << remove_index << ")\n";

    peaks.erase(peaks.begin() + idx_remove);
    sort_and_reorder(peaks);
    return true;
}

static bool add_one_missing_peak(std::vector<Peak> &peaks,
                                 TH1D *hist,
                                 int add_index,
                                 double threshold_abs_start,
                                 double threshold_abs_min,
                                 double threshold_step_abs)
{
    if (peaks.size() < 2)
        return false;
    sort_and_reorder(peaks);

    if (add_index < 0 || add_index + 1 >= (int)peaks.size())
        return false;

    int left = (int)std::round(peaks[add_index].x);
    int right = (int)std::round(peaks[add_index + 1].x);

    std::cout << "Gap too wide: " << (right - left)
              << " (between x=" << peaks[add_index].x
              << " and " << peaks[add_index + 1].x << ")\n";

    for (double thr = threshold_abs_start; thr >= threshold_abs_min; thr -= threshold_step_abs)
    {
        auto cand = find_peaks_in_range(hist, left + 1, right - 1, thr);
        if (!cand.empty())
        {
            auto best_it = std::max_element(
                cand.begin(), cand.end(),
                [](const Peak &a, const Peak &b)
                { return a.y < b.y; });

            Peak best = *best_it;

            std::cout << "  Found missing peak at x=" << best.x
                      << " y=" << best.y
                      << " with thr=" << thr << "\n";

            peaks.push_back(best);
            sort_and_reorder(peaks);
            return true;
        }
    }

    std::cout << "  No peak found in this gap even after lowering threshold.\n";
    return false;
}

static void adjust_peaks(std::vector<Peak> &peaks, TH1D *hist, double max_y, double base_threshold_frac)
{
    const int max_iter = 200;

    for (int it = 0; it < max_iter; ++it)
    {
        JudgePeak res;
        judge_peaks(peaks, res);
        if (peaks.size() < 2)
            break;

        bool changed = false;

        if (!res.RemovePeakIndices.empty())
        {
            std::sort(res.RemovePeakIndices.begin(), res.RemovePeakIndices.end());
            for (int k = (int)res.RemovePeakIndices.size() - 1; k >= 0; --k)
            {
                if (remove_one_close_pair(peaks, res.RemovePeakIndices[k]))
                    changed = true;
            }
        }

        // Only add if we did not remove in this iteration (keeps it stable)
        if (!changed && !res.AddPeakIndices.empty())
        {
            double thr_start = base_threshold_frac * max_y;
            double thr_min = 0.05 * max_y;
            double thr_step = 0.01 * max_y;

            int idx = res.AddPeakIndices.front();
            if (add_one_missing_peak(peaks, hist, idx, thr_start, thr_min, thr_step))
                changed = true;
        }

        if (!changed)
        {
            std::cout << "Peaks stable after " << it << " iterations.\n";
            break;
        }
    }
}

// ---------- one run: given expected peak count, find threshold and adjust peaks ----------
static std::vector<Peak> run_once(TH1D *h1, int NumPeakExpected, double &out_threshold_frac)
{
    const double max_x = h1->GetMaximum();

    // 1) binary search threshold to match expected peaks
    double threshold_frac = 1.0;
    double thr_hi = 1.0;
    double thr_lo = 0.0;

    int NumPeak = (int)number_of_peaks_above_threshold(h1, threshold_frac * max_x).size();
    int iter = 0;

    while (NumPeak != NumPeakExpected)
    {
        if (iter >= 200)
        {
            std::cout << "Max iterations reached in threshold search.\n";
            break;
        }

        if (NumPeak > NumPeakExpected)
        {
            // too many peaks -> threshold too low -> increase
            thr_lo = threshold_frac;
            threshold_frac = 0.5 * (threshold_frac + thr_hi);
        }
        else
        {
            // too few peaks -> threshold too high -> decrease
            thr_hi = threshold_frac;
            threshold_frac = 0.5 * (threshold_frac + thr_lo);
        }

        NumPeak = (int)number_of_peaks_above_threshold(h1, threshold_frac * max_x).size();
        ++iter;
    }

    // 2) initial peaks + adjust
    std::vector<Peak> peaks = number_of_peaks_above_threshold(h1, threshold_frac * max_x);
    sort_and_reorder(peaks);

    adjust_peaks(peaks, h1, max_x, threshold_frac);

    out_threshold_frac = threshold_frac;
    return peaks;
}

// ---------- outer-most IO + iteration ----------
int PeakScan(TH1D *h1, std::vector<Peak> &best_peaks, int expected=70)
{
    // ---------------- iteration on expected peak count ----------------
    // int expected = 70; // your initial guess
    const int max_outer_iter = 50;

    // std::vector<Peak> best_peaks;
    double best_thr = 0.0;

    for (int it = 0; it < max_outer_iter; ++it)
    {
        double thr_frac = 0.0;
        std::vector<Peak> peaks = run_once(h1, expected, thr_frac);

        int found = (int)peaks.size();
        std::cout << "[outer " << it << "] expected=" << expected
                  << " found=" << found
                  << " threshold_frac=" << thr_frac << "\n";

        best_peaks = std::move(peaks);
        best_thr = thr_frac;

        if (found == expected)
        {
            std::cout << "Stable peak count reached: " << found << "\n";
            break;
        }

        expected = found; // update expectation and repeat
    }

    sort_and_reorder(best_peaks);


    return 0;
}

void convertPeaksToBinEdges(const std::vector<Peak> &peaks,
                            std::vector<double> &binEdges)
{
    binEdges.clear();

    const size_t N = peaks.size();
    if (N == 0) return;

    // If only one peak, cannot define spacing reliably
    if (N == 1)
    {
        // Arbitrary small symmetric region
        binEdges.push_back(peaks[0].x - 0.5);
        binEdges.push_back(peaks[0].x + 0.5);
        return;
    }

    // Make sure peaks are sorted (important!)
    // (Assumes you already call sort_and_reorder before)
    // If unsure, you can uncomment:
    // std::vector<Peak> sorted = peaks;
    // sort_and_reorder(sorted);

    // ---------- first boundary ----------
    double first = peaks[0].x
                 - 0.5 * (peaks[1].x - peaks[0].x);

    binEdges.push_back(first);

    // ---------- internal boundaries ----------
    for (size_t i = 0; i < N - 1; ++i)
    {
        double mid = 0.5 * (peaks[i].x + peaks[i+1].x);
        binEdges.push_back(mid);
    }

    // ---------- last boundary ----------
    double last = peaks[N-1].x
                + 0.5 * (peaks[N-1].x - peaks[N-2].x);

    binEdges.push_back(last);
}

void test_PeakScan(int run_number, bool projX, int stationID, int expected_peak_count = 70)
{

    // ---------------- input layer (ONLY ONCE) ----------------
    TFile *f = TFile::Open(Form("output/run%d_ScanXY.root", run_number), "READ");
    if (!f || f->IsZombie())
    {
        std::cerr << "Error: cannot open file.\n";
        return;
    }

    TH2D *h2d = dynamic_cast<TH2D *>(f->Get(Form("ScanXY/Pos2D_H%dV%d", stationID, stationID)));
    if (!h2d)
    {
        std::cerr << "Error: cannot find histogram ScanXY/Pos2D_H1V1\n";
        f->Close();
        return;
    }

    TH1D *h1 = projX ? h2d->ProjectionX("h1") : h2d->ProjectionY("h1");
    if (!h1)
    {
        std::cerr << "Error: ProjectionX failed.\n";
        f->Close();
        return;
    }

    // Run the peak scan
    std::vector<Peak> best_peaks;
    PeakScan(h1, best_peaks, expected_peak_count);

    // ---------------- output layer (ONLY ONCE) ----------------
    std::cout << "Final number of peaks found: " << best_peaks.size() << "\n";

    TCanvas *c = new TCanvas("c", "Peak Finding", 800, 600);
    h1->Draw();

    for (const auto &p : best_peaks)
    {
        double x = h1->GetXaxis()->GetBinCenter((int)std::round(p.x));
        TLine *line = new TLine(x, 0, x, p.y);
        line->SetLineColor(kRed);
        line->Draw("same");
    }

    c->SaveAs(Form("output/run%d_Peaks.png", run_number));

    f->Close();
}