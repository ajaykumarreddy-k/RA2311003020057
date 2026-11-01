# Backpropagation Derivation & Numerical Verification (Handwriting Guide)

> **Format Note for Candidate:** Copy the contents below by hand onto two physical pages (Page 1 and Page 2). All derivations, equations, and exact numerical steps are provided below.

---

# ==================== PAGE 1 ====================
## 1. Network Architecture & Problem Formulation

### 1.1 Objective and Dataset
- **Single Training Sample:** Input $x = 1.0$, Target output $y = 2.0$.
- **Network Topology:** 1 Input $\to$ 1 Hidden Neuron (tanh activation) $\to$ 1 Linear Output Neuron.
- **Model Parameters:**
  - Hidden weight: $w \in \mathbb{R}$ (initial $w_0 = 0.5$)
  - Hidden bias: $b \in \mathbb{R}$ (initial $b_0 = 0.0$)
  - Output weight: $v \in \mathbb{R}$ (initial $v_0 = 1.0$)
  - Output bias: $c \in \mathbb{R}$ (initial $c_0 = 0.0$)
  - Learning rate: $\eta = 0.1$

### 1.2 Mathematical Formulation
1. **Hidden Pre-activation:**
   $$z = w \cdot x + b$$
2. **Hidden Activation (Hyperbolic Tangent):**
   $$h = \tanh(z) = \frac{e^z - e^{-z}}{e^z + e^{-z}}$$
   *Derivative Property:*
   $$\frac{dh}{dz} = \frac{d}{dz}\tanh(z) = 1 - \tanh^2(z) = 1 - h^2$$
3. **Network Prediction:**
   $$\hat{y} = v \cdot h + c$$
4. **Loss Function (Mean Squared Error / Half Residual Square):**
   $$J = \frac{1}{2} (\hat{y} - y)^2$$

---

## 2. Forward Pass at Step 0 (Numerical Evaluation)

Substituting initial parameters $w = 0.5, b = 0, v = 1, c = 0$ with $x = 1, y = 2$:

1. **Calculate $z$:**
   $$z = (0.5)(1.0) + 0.0 = 0.5$$

2. **Calculate $h$:**
   $$h = \tanh(0.5) = \frac{e^{0.5} - e^{-0.5}}{e^{0.5} + e^{-0.5}} \approx \frac{1.64872127 - 0.60653066}{1.64872127 + 0.60653066} = \frac{1.04219061}{2.25525193} \approx 0.46211716$$

3. **Calculate Output $\hat{y}$:**
   $$\hat{y} = (1.0)(0.46211716) + 0.0 = 0.46211716$$

4. **Calculate Prediction Error $e$ and Loss $J$:**
   $$e = \hat{y} - y = 0.46211716 - 2.0 = -1.53788284$$
   $$J = \frac{1}{2} (-1.53788284)^2 = \frac{1}{2} (2.36508364) \approx 1.18254182$$

---
*(End of Page 1 - Turn page over to Page 2)*
\newpage
---

# ==================== PAGE 2 ====================
## 3. Backward Pass (Analytical Chain Rule Derivation)

We compute the partial derivatives of $J$ with respect to each parameter $(c, v, w, b)$ using the chain rule.

### 3.1 Output Layer Gradients ($c$ and $v$)
- **Sensitivity of Loss wrt Prediction:**
  $$\frac{\partial J}{\partial \hat{y}} = \frac{\partial}{\partial \hat{y}} \left[ \frac{1}{2}(\hat{y} - y)^2 \right] = \hat{y} - y = e$$

- **Gradient wrt Output Bias $c$:**
  $$\frac{\partial J}{\partial c} = \frac{\partial J}{\partial \hat{y}} \cdot \frac{\partial \hat{y}}{\partial c} = (\hat{y} - y) \cdot (1) = \mathbf{\hat{y} - y}$$

- **Gradient wrt Output Weight $v$:**
  $$\frac{\partial J}{\partial v} = \frac{\partial J}{\partial \hat{y}} \cdot \frac{\partial \hat{y}}{\partial v} = (\hat{y} - y) \cdot (h) = \mathbf{(\hat{y} - y) \, h}$$

### 3.2 Hidden Layer Gradients ($w$ and $b$)
- **Backpropagated error to hidden state $h$:**
  $$\frac{\partial J}{\partial h} = \frac{\partial J}{\partial \hat{y}} \cdot \frac{\partial \hat{y}}{\partial h} = (\hat{y} - y) \cdot v$$

- **Hidden neuron error sensitivity $\delta \equiv \frac{\partial J}{\partial z}$:**
  $$\delta = \frac{\partial J}{\partial h} \cdot \frac{dh}{dz} = [(\hat{y} - y) \cdot v] \cdot (1 - h^2)$$

- **Gradient wrt Hidden Weight $w$:**
  $$\frac{\partial J}{\partial w} = \frac{\partial J}{\partial z} \cdot \frac{\partial z}{\partial w} = \delta \cdot x = \mathbf{(\hat{y} - y) \, v \, (1 - h^2) \, x}$$

- **Gradient wrt Hidden Bias $b$:**
  $$\frac{\partial J}{\partial b} = \frac{\partial J}{\partial z} \cdot \frac{\partial z}{\partial b} = \delta \cdot 1 = \mathbf{(\hat{y} - y) \, v \, (1 - h^2)}$$

---

## 4. Step 0 Numerical Gradients & Gradient Check ($\epsilon = 10^{-6}$)

Using $e = -1.53788284$, $h = 0.46211716$, $v = 1.0$, $x = 1.0$:
- **Activation slope:** $1 - h^2 = 1 - (0.46211716)^2 = 1 - 0.21355227 = 0.78644773$
- **Hidden delta:** $\delta = (-1.53788284)(1.0)(0.78644773) = -1.20946459$

| Parameter $\theta$ | Analytic Gradient $\frac{\partial J}{\partial \theta}$ | Finite Difference $\frac{J(\theta+\epsilon)-J(\theta-\epsilon)}{2\epsilon}$ | Absolute Error | Status ($|\text{Error}| \le 10^{-5}$) |
|---|---|---|---|---|
| $c$ | **$-1.53788284$** | $-1.53788284$ | $3.5 \times 10^{-11}$ | **PASSED** |
| $v$ | **$-0.71068205$** | $-0.71068205$ | $1.6 \times 10^{-11}$ | **PASSED** |
| $w$ | **$-1.20946459$** | $-1.20946459$ | $2.8 \times 10^{-11}$ | **PASSED** |
| $b$ | **$-1.20946459$** | $-1.20946459$ | $2.8 \times 10^{-11}$ | **PASSED** |

---

## 5. Simultaneous Parameter Update (Step 1)
Using learning rate $\eta = 0.1$:
$$\theta^{(1)} = \theta^{(0)} - \eta \, \frac{\partial J}{\partial \theta}$$

1. **Hidden Weight $w$:**
   $$w^{(1)} = 0.5 - (0.1)(-1.20946459) = 0.5 + 0.12094646 = \mathbf{0.62094646}$$
2. **Hidden Bias $b$:**
   $$b^{(1)} = 0.0 - (0.1)(-1.20946459) = \mathbf{0.12094646}$$
3. **Output Weight $v$:**
   $$v^{(1)} = 1.0 - (0.1)(-0.71068205) = 1.0 + 0.07106821 = \mathbf{1.07106821}$$
4. **Output Bias $c$:**
   $$c^{(1)} = 0.0 - (0.1)(-1.53788284) = \mathbf{0.15378828}$$

*Verification after Step 1:*
$z^{(1)} = (0.62094646)(1) + 0.12094646 = 0.74189292 \implies h^{(1)} = \tanh(0.74189292) \approx 0.63024846$
$\hat{y}^{(1)} = (1.07106821)(0.63024846) + 0.15378828 \approx 0.82882200$
$J^{(1)} = \frac{1}{2}(0.82882200 - 2.0)^2 \approx 0.68582894$ (**Loss decreased by $42.0\%$ in step 1!**)
---
*(End of Page 2)*
