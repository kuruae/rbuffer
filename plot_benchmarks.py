import matplotlib.pyplot as plt
import numpy as np

# Data
labels = ['p50', 'p99', 'p999']
my_spsc = [517, 582, 667]
boost_spsc = [433, 573, 655]
fs_spsc = [611, 703, 845]
lock_spsc = [1501, 2566, 2803]

x = np.arange(len(labels))  # the label locations
width = 0.2  # the width of the bars

# Modern color scheme
colors = {
    'Boost': '#1f77b4',       # Blue
    'My SPSC': '#2ca02c',     # Green
    'False Sharing': '#ff7f0e', # Orange
    'Lock-based': '#d62728'   # Red
}

fig, ax = plt.subplots(figsize=(10, 6))

# Plot bars
rects1 = ax.bar(x - width*1.5, boost_spsc, width, label='Boost SPSC', color=colors['Boost'], edgecolor='white', linewidth=1)
rects2 = ax.bar(x - width*0.5, my_spsc, width, label='My SPSC', color=colors['My SPSC'], edgecolor='white', linewidth=1)
rects3 = ax.bar(x + width*0.5, fs_spsc, width, label='False sharing SPSC', color=colors['False Sharing'], edgecolor='white', linewidth=1)
rects4 = ax.bar(x + width*1.5, lock_spsc, width, label='Lock-based SPSC', color=colors['Lock-based'], edgecolor='white', linewidth=1)

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Latency (CPU Cycles)', fontsize=12, fontweight='bold')
ax.set_title('SPSC Queue Latency Comparison', fontsize=16, fontweight='bold', pad=20)
ax.set_xticks(x)
ax.set_xticklabels(labels, fontsize=12)
ax.legend(fontsize=11)

# Add gridlines for readability
ax.grid(axis='y', linestyle='--', alpha=0.7)

# Remove top and right spines
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)

# Add values on top of bars
def autolabel(rects):
    """Attach a text label above each bar in *rects*, displaying its height."""
    for rect in rects:
        height = rect.get_height()
        ax.annotate('{}'.format(height),
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),  # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=9)

autolabel(rects1)
autolabel(rects2)
autolabel(rects3)
autolabel(rects4)

fig.tight_layout()

# Save as PNG
plt.savefig('benchmark_results.png', dpi=300, bbox_inches='tight')
print("Chart successfully saved as benchmark_results.png")
